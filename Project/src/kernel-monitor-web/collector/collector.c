/* Linux user-space collector. One JSON object per line on stdout.
 * Node reads stdout through an IPC pipe. Errors go to stderr.
 * --once takes a baseline, waits one second, and prints one sample.
 */
#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

typedef unsigned long long U64;
typedef struct { U64 total, idle; } Cpu;
typedef struct {
    U64 total, available, buffers, cached, swap_total, swap_free;
} Memory;
typedef struct {
    int pid;
    char name[256], state;
    U64 ticks, start_ticks, rss_kb, threads;
    double cpu;
} Process;

static volatile sig_atomic_t running = 1;
static void stop(int sig) { (void)sig; running = 0; }
static void fail(const char *operation) { perror(operation); exit(EXIT_FAILURE); }

/* Explicit open/read/close. Loop because read() can return partial data.
 * A bounded prefix is enough for the /proc fields used in this program.
 */
static int read_text(const char *path, char *buf, size_t capacity) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) return -1;
    size_t used = 0;
    while (used < capacity - 1) {
        ssize_t n = read(fd, buf + used, capacity - 1 - used);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) { int saved = errno; close(fd); errno = saved; return -1; }
        if (n == 0) break;
        used += (size_t)n;
    }
    buf[used] = '\0';
    close(fd);
    return 0;
}

static double seconds(clockid_t clock) {
    struct timespec ts;
    if (clock_gettime(clock, &ts) != 0) fail("clock_gettime");
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static Cpu read_cpu(void) {
    char buf[1024];
    U64 user, nice, system, idle, wait = 0, irq = 0, softirq = 0, steal = 0;
    if (read_text("/proc/stat", buf, sizeof buf) != 0) fail("/proc/stat");
    if (sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &wait, &irq, &softirq, &steal) < 4) {
        fprintf(stderr, "Cannot parse aggregate CPU counters\n"); exit(EXIT_FAILURE);
    }
    /* guest/guest_nice are already included in user/nice; do not add them. */
    Cpu result = {user + nice + system + idle + wait + irq + softirq + steal,
                  idle + wait};
    return result;
}

static Memory read_memory(const struct sysinfo *system) {
    char buf[16384], *save = NULL;
    Memory mem = {
        .total = (U64)system->totalram * system->mem_unit,
        .available = (U64)system->freeram * system->mem_unit,
        .buffers = (U64)system->bufferram * system->mem_unit,
        .swap_total = (U64)system->totalswap * system->mem_unit,
        .swap_free = (U64)system->freeswap * system->mem_unit
    };
    if (read_text("/proc/meminfo", buf, sizeof buf) != 0) fail("/proc/meminfo");
    bool has_available = false;
    U64 free_bytes = mem.available;
    for (char *line = strtok_r(buf, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
        char key[64]; U64 kb;
        if (sscanf(line, "%63[^:]: %llu kB", key, &kb) != 2) continue;
        U64 bytes = kb * 1024ULL;
        if (!strcmp(key, "MemTotal")) mem.total = bytes;
        else if (!strcmp(key, "MemAvailable")) { mem.available = bytes; has_available = true; }
        else if (!strcmp(key, "MemFree")) free_bytes = bytes;
        else if (!strcmp(key, "Buffers")) mem.buffers = bytes;
        else if (!strcmp(key, "Cached")) mem.cached = bytes;
        else if (!strcmp(key, "SwapTotal")) mem.swap_total = bytes;
        else if (!strcmp(key, "SwapFree")) mem.swap_free = bytes;
    }
    if (!has_available) mem.available = free_bytes;
    if (mem.available > mem.total) mem.available = mem.total;
    return mem;
}

static bool read_process(int pid, Process *p) {
    char path[64], buf[16384], *save = NULL;
    memset(p, 0, sizeof *p);
    p->pid = pid; p->state = '?'; strcpy(p->name, "?");
    snprintf(path, sizeof path, "/proc/%d/status", pid);
    if (read_text(path, buf, sizeof buf) != 0) return false;
    for (char *line = strtok_r(buf, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
        if (!strncmp(line, "Name:", 5)) {
            char *name = line + 5;
            while (isspace((unsigned char)*name)) ++name;
            snprintf(p->name, sizeof p->name, "%s", name);
        } else if (!strncmp(line, "State:", 6)) sscanf(line + 6, " %c", &p->state);
        else if (!strncmp(line, "VmRSS:", 6)) sscanf(line + 6, "%llu", &p->rss_kb);
        else if (!strncmp(line, "Threads:", 8)) sscanf(line + 8, "%llu", &p->threads);
    }
    snprintf(path, sizeof path, "/proc/%d/stat", pid);
    if (read_text(path, buf, sizeof buf) != 0) return false;
    /* comm can contain spaces and parentheses; start after its final ')'. */
    char *end = strrchr(buf, ')');
    if (!end) return false;
    save = NULL;
    int field = 3;
    for (char *token = strtok_r(end + 1, " \n", &save); token;
         token = strtok_r(NULL, " \n", &save), ++field) {
        if (field == 14 || field == 15) p->ticks += strtoull(token, NULL, 10);
        if (field == 22) { p->start_ticks = strtoull(token, NULL, 10); return true; }
    }
    return false;
}

static int by_pid(const void *a, const void *b) {
    int x = ((const Process *)a)->pid, y = ((const Process *)b)->pid;
    return (x > y) - (x < y);
}

static Process *scan_processes(size_t *count) {
    DIR *dir = opendir("/proc");
    if (!dir) fail("opendir /proc");
    size_t used = 0, capacity = 128;
    Process *rows = malloc(capacity * sizeof *rows);
    if (!rows) fail("malloc");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *end;
        long pid = strtol(entry->d_name, &end, 10);
        if (end == entry->d_name || *end || pid <= 0 || pid > 2147483647L) continue;
        Process p;
        /* Processes may exit or deny access while we enumerate them. */
        if (!read_process((int)pid, &p)) continue;
        if (used == capacity) {
            capacity *= 2;
            Process *larger = realloc(rows, capacity * sizeof *rows);
            if (!larger) fail("realloc");
            rows = larger;
        }
        rows[used++] = p;
    }
    closedir(dir);
    qsort(rows, used, sizeof *rows, by_pid);
    *count = used;
    return rows;
}

static int by_usage(const void *a, const void *b) {
    const Process *x = a, *y = b;
    if (x->cpu != y->cpu) return (x->cpu < y->cpu) ? 1 : -1;
    return (x->rss_kb < y->rss_kb) - (x->rss_kb > y->rss_kb);
}

/* Escape names so quotes/control bytes cannot corrupt the JSON. */
static void json_string(const char *text) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
        if (*p == '"' || *p == '\\') printf("\\%c", *p);
        else if (*p < 32 || *p >= 127) printf("\\u%04x", (unsigned)*p);
        else putchar(*p);
    }
    putchar('"');
}

static void emit(double cpu, Memory m, Process *rows, size_t count,
                 const struct sysinfo *info, const struct rusage *usage) {
    U64 used = m.total - m.available, threads = 0;
    for (size_t i = 0; i < count; ++i) threads += rows[i].threads;
    printf("{\"t\":%.0f,\"cpuPercent\":%.1f,\"usingRealProc\":true,\"collector\":\"c\",",
           seconds(CLOCK_REALTIME) * 1000, cpu);
    printf("\"memory\":{\"totalBytes\":%llu,\"usedBytes\":%llu,\"freeBytes\":%llu,"
           "\"buffersBytes\":%llu,\"cachedBytes\":%llu,\"swapTotalBytes\":%llu,"
           "\"swapFreeBytes\":%llu,\"usedPercent\":%.1f},",
           m.total, used, m.available, m.buffers, m.cached, m.swap_total, m.swap_free,
           m.total ? (double)used / m.total * 100 : 0);
    printf("\"uptimeSeconds\":%ld,\"loadAverage\":[%.3f,%.3f,%.3f],"
           "\"totalProcesses\":%zu,\"totalThreads\":%llu,",
           info->uptime, info->loads[0] / 65536.0, info->loads[1] / 65536.0,
           info->loads[2] / 65536.0, count, threads);
    /* RUSAGE_SELF measures this collector, not every process. */
    printf("\"collectorUsage\":{\"userCpuSeconds\":%.6f,\"systemCpuSeconds\":%.6f,"
           "\"maxRssKb\":%ld,\"voluntaryContextSwitches\":%ld,"
           "\"involuntaryContextSwitches\":%ld},",
           usage->ru_utime.tv_sec + usage->ru_utime.tv_usec / 1e6,
           usage->ru_stime.tv_sec + usage->ru_stime.tv_usec / 1e6,
           usage->ru_maxrss, usage->ru_nvcsw, usage->ru_nivcsw);
    /* Preserve PID ordering in rows for the next sample's bsearch(). */
    Process *ranked = malloc((count ? count : 1) * sizeof *ranked);
    if (!ranked) fail("malloc");
    memcpy(ranked, rows, count * sizeof *ranked);
    qsort(ranked, count, sizeof *ranked, by_usage);
    printf("\"processes\":[");
    for (size_t i = 0; i < count && i < 30; ++i) {
        Process *p = &ranked[i];
        if (i) putchar(',');
        printf("{\"pid\":%d,\"name\":", p->pid); json_string(p->name);
        char state[2] = {p->state, '\0'};
        printf(",\"state\":"); json_string(state);
        printf(",\"memMb\":%.1f,\"threads\":%llu,\"cpuPercent\":%.1f}",
               p->rss_kb / 1024.0, p->threads, p->cpu);
    }
    free(ranked);
    puts("]}");
    if (fflush(stdout) == EOF) fail("stdout pipe");
}

int main(int argc, char **argv) {
    bool once = argc == 2 && !strcmp(argv[1], "--once");
    if (argc > 1 && !once) {
        fprintf(stderr, "Usage: %s [--once]\n", argv[0]); return EXIT_FAILURE;
    }
    signal(SIGINT, stop); signal(SIGTERM, stop);
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    if (ticks_per_second <= 0) fail("sysconf _SC_CLK_TCK");
    Cpu previous_cpu = read_cpu();
    size_t previous_count;
    Process *previous = scan_processes(&previous_count);
    double previous_time = seconds(CLOCK_MONOTONIC);

    while (running) {
        struct timespec delay = {1, 0};
        while (nanosleep(&delay, &delay) == -1 && errno == EINTR && running) {}
        if (!running) break;
        Cpu current_cpu = read_cpu();
        size_t count;
        Process *rows = scan_processes(&count);
        double now = seconds(CLOCK_MONOTONIC), elapsed = now - previous_time;
        double cpu = 0;
        if (current_cpu.total > previous_cpu.total && current_cpu.idle >= previous_cpu.idle) {
            double total_delta = (double)(current_cpu.total - previous_cpu.total);
            double idle_delta = (double)(current_cpu.idle - previous_cpu.idle);
            cpu = 100.0 * (1.0 - idle_delta / total_delta);
            if (cpu < 0) cpu = 0;
            if (cpu > 100) cpu = 100;
        }
        for (size_t i = 0; i < count; ++i) {
            Process *old = bsearch(&rows[i], previous, previous_count, sizeof *previous, by_pid);
            /* New/reused PIDs establish a baseline; don't report lifetime usage. */
            if (old && old->start_ticks == rows[i].start_ticks && rows[i].ticks >= old->ticks && elapsed > 0)
                rows[i].cpu = (double)(rows[i].ticks - old->ticks) / ticks_per_second / elapsed * 100;
        }
        struct sysinfo info;
        struct rusage usage;
        if (sysinfo(&info) == -1) fail("sysinfo");
        if (getrusage(RUSAGE_SELF, &usage) == -1) fail("getrusage");
        emit(cpu, read_memory(&info), rows, count, &info, &usage);
        free(previous);
        previous = rows; previous_count = count;
        previous_cpu = current_cpu; previous_time = now;
        if (once) break;
    }
    free(previous);
    return EXIT_SUCCESS;
}
