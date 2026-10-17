#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "queue.h"
#include "search.h"

// Forward declaration
void escape_json(const char* src, char* dest, size_t max_dest);

typedef struct {
    WorkerTelemetry* telemetry;
    SearchStats* stats;
    int num_threads;
    bool* stop_flag;
    double start_time;
    SearchOptions* options;
} TelemetryArg;

// High-resolution timestamp in milliseconds
static double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) * 1000.0 + (double)(tv.tv_usec) / 1000.0;
}

// Background thread for streaming live telemetry pulses
void* telemetry_reporter_fn(void* arg) {
    TelemetryArg* targ = (TelemetryArg*)arg;
    char escaped_cur_file[MAX_PATH_LEN * 2];

    while (!(*(targ->stop_flag))) {
        usleep(60000); // 60ms pulse interval (~16 FPS telemetry rate)

        if (*(targ->stop_flag)) break;

        double elapsed = get_time_ms() - targ->start_time;

        pthread_mutex_lock(&targ->stats->print_lock);
        printf("{\"type\":\"telemetry\",\"elapsed_ms\":%.2f,\"total_files\":%lu,\"total_dirs\":%lu,\"total_matches\":%lu,\"threads\":[",
               elapsed, targ->stats->total_files, targ->stats->total_dirs, targ->stats->total_matches);

        for (int i = 0; i < targ->num_threads; i++) {
            WorkerTelemetry* t = &targ->telemetry[i];
            pthread_mutex_lock(&t->worker_lock);
            escape_json(t->current_file, escaped_cur_file, sizeof(escaped_cur_file));
            printf("%s{\"id\":%d,\"status\":\"%s\",\"files\":%lu,\"matches\":%lu,\"current_file\":\"%s\"}",
                   (i > 0 ? "," : ""),
                   t->thread_id,
                   t->status,
                   t->files_scanned,
                   t->matches_found,
                   escaped_cur_file);
            pthread_mutex_unlock(&t->worker_lock);
        }
        printf("]}\n");
        fflush(stdout);
        pthread_mutex_unlock(&targ->stats->print_lock);
    }
    return NULL;
}

void print_usage(const char* prog) {
    fprintf(stderr, "Parallel File Search Engine (C POSIX Threads)\n");
    fprintf(stderr, "Usage: %s [options]\n", prog);
    fprintf(stderr, "  -d <dir>             Root directory to search (default: .)\n");
    fprintf(stderr, "  -p <pattern>         Search pattern/query\n");
    fprintf(stderr, "  -t <threads>         Number of worker threads (default: 4)\n");
    fprintf(stderr, "  --name               Search in filenames\n");
    fprintf(stderr, "  --content            Search inside file content\n");
    fprintf(stderr, "  --case-sensitive     Enable case-sensitive matching\n");
    fprintf(stderr, "  --ext <extensions>   Comma-separated extensions (.c,.h,.py,.txt)\n");
    fprintf(stderr, "  --max-depth <n>      Max directory recursion depth\n");
    fprintf(stderr, "  --stream             Output JSON event stream for Web UI\n");
    fprintf(stderr, "  -h, --help           Show this help message\n");
}

int main(int argc, char* argv[]) {
    SearchOptions opts;
    strncpy(opts.root_dir, ".", sizeof(opts.root_dir));
    opts.pattern[0] = '\0';
    opts.search_name = false;
    opts.search_content = false;
    opts.case_sensitive = false;
    opts.stream_mode = false;
    opts.num_threads = 4;
    opts.max_depth = 0;
    opts.ext_filter[0] = '\0';

    // Parse CLI arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(opts.root_dir, argv[++i], sizeof(opts.root_dir) - 1);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            strncpy(opts.pattern, argv[++i], sizeof(opts.pattern) - 1);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            opts.num_threads = atoi(argv[++i]);
            if (opts.num_threads <= 0) opts.num_threads = 1;
            if (opts.num_threads > 64) opts.num_threads = 64;
        } else if (strcmp(argv[i], "--name") == 0) {
            opts.search_name = true;
        } else if (strcmp(argv[i], "--content") == 0) {
            opts.search_content = true;
        } else if (strcmp(argv[i], "--case-sensitive") == 0) {
            opts.case_sensitive = true;
        } else if (strcmp(argv[i], "--stream") == 0) {
            opts.stream_mode = true;
        } else if (strcmp(argv[i], "--ext") == 0 && i + 1 < argc) {
            strncpy(opts.ext_filter, argv[++i], sizeof(opts.ext_filter) - 1);
        } else if (strcmp(argv[i], "--max-depth") == 0 && i + 1 < argc) {
            opts.max_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Default: if neither specified, search both
    if (!opts.search_name && !opts.search_content) {
        opts.search_name = true;
        opts.search_content = true;
    }

    // Initialize shared TaskQueue
    TaskQueue queue;
    queue_init(&queue);

    // Initialize stats
    SearchStats stats;
    stats.total_files = 0;
    stats.total_dirs = 0;
    stats.total_bytes = 0;
    stats.total_matches = 0;
    pthread_mutex_init(&stats.stats_lock, NULL);
    pthread_mutex_init(&stats.print_lock, NULL);

    // Allocate telemetry and worker args
    WorkerTelemetry* telem = (WorkerTelemetry*)calloc(opts.num_threads, sizeof(WorkerTelemetry));
    WorkerArg* wargs = (WorkerArg*)calloc(opts.num_threads, sizeof(WorkerArg));
    pthread_t* threads = (pthread_t*)calloc(opts.num_threads, sizeof(pthread_t));

    for (int i = 0; i < opts.num_threads; i++) {
        telem[i].thread_id = i;
        telem[i].files_scanned = 0;
        telem[i].bytes_scanned = 0;
        telem[i].matches_found = 0;
        telem[i].current_file[0] = '\0';
        strncpy(telem[i].status, "IDLE", 31);
        pthread_mutex_init(&telem[i].worker_lock, NULL);

        wargs[i].thread_id = i;
        wargs[i].queue = &queue;
        wargs[i].options = &opts;
        wargs[i].telemetry = &telem[i];
        wargs[i].stats = &stats;
    }

    double start_time = get_time_ms();

    // Push initial root directory task
    queue_push(&queue, opts.root_dir, TASK_DIRECTORY, 0);

    // Start worker threads
    for (int i = 0; i < opts.num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread_fn, &wargs[i]) != 0) {
            fprintf(stderr, "Error creating worker thread %d\n", i);
        }
    }

    // Start telemetry background thread if stream mode
    pthread_t telem_thread;
    bool stop_telem = false;
    TelemetryArg targ;
    if (opts.stream_mode) {
        targ.telemetry = telem;
        targ.stats = &stats;
        targ.num_threads = opts.num_threads;
        targ.stop_flag = &stop_telem;
        targ.start_time = start_time;
        targ.options = &opts;
        pthread_create(&telem_thread, NULL, telemetry_reporter_fn, &targ);
    }

    // Wait for all worker threads to complete
    for (int i = 0; i < opts.num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double end_time = get_time_ms();
    double duration_ms = end_time - start_time;

    // Stop telemetry thread
    if (opts.stream_mode) {
        stop_telem = true;
        pthread_join(telem_thread, NULL);
    }

    // Output final summary JSON
    if (opts.stream_mode) {
        pthread_mutex_lock(&stats.print_lock);
        printf("{\"type\":\"done\",\"threads\":%d,\"total_files\":%lu,\"total_dirs\":%lu,\"total_matches\":%lu,\"duration_ms\":%.2f}\n",
               opts.num_threads, stats.total_files, stats.total_dirs, stats.total_matches, duration_ms);
        fflush(stdout);
        pthread_mutex_unlock(&stats.print_lock);
    } else {
        printf("Search completed in %.2f ms\n", duration_ms);
        printf("Threads: %d | Directories: %lu | Files Scanned: %lu | Matches: %lu\n",
               opts.num_threads, stats.total_dirs, stats.total_files, stats.total_matches);
    }

    // Cleanup
    for (int i = 0; i < opts.num_threads; i++) {
        pthread_mutex_destroy(&telem[i].worker_lock);
    }
    pthread_mutex_destroy(&stats.stats_lock);
    pthread_mutex_destroy(&stats.print_lock);
    queue_destroy(&queue);

    free(threads);
    free(wargs);
    free(telem);

    return 0;
}
