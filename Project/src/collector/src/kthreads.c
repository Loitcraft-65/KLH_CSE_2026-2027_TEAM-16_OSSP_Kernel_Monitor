/**
 * ============================================================================
 * kthreads.c - Linux Ring-0 Kernel Threads Inspector
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include "../include/kthreads.h"

static int parse_kernel_thread(int pid, KernelThreadInfo *info) {
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    info->pid = pid;
    info->ppid = 0;
    info->name[0] = '\0';
    info->state = '?';
    info->voluntary_ctxt = 0;
    info->nonvoluntary_ctxt = 0;

    int has_vmsize = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Name:", 5) == 0) {
            char *p = line + 5;
            while (*p == ' ' || *p == '\t') p++;
            size_t len = strlen(p);
            if (len > 0 && p[len - 1] == '\n') p[len - 1] = '\0';
            snprintf(info->name, sizeof(info->name), "%s", p);
        } else if (strncmp(line, "State:", 6) == 0) {
            char *p = line + 6;
            while (*p == ' ' || *p == '\t') p++;
            info->state = *p;
        } else if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 5, "%d", &info->ppid);
        } else if (strncmp(line, "VmSize:", 7) == 0) {
            has_vmsize = 1;
        } else if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
            sscanf(line + 24, "%ld", &info->voluntary_ctxt);
        } else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
            sscanf(line + 27, "%ld", &info->nonvoluntary_ctxt);
        }
    }
    fclose(fp);

    // Filter STRICTLY to Kernel Threads:
    // In Linux kernel, PID 2 is kthreadd.
    // All kernel threads are children of kthreadd (PPID == 2) or have NO user VmSize!
    if (pid == 2 || info->ppid == 2 || !has_vmsize) {
        return 0; // Valid kernel thread
    }
    return -1; // Skip user space processes
}

static int compare_kthreads(const void *a, const void *b) {
    const KernelThreadInfo *ta = (const KernelThreadInfo *)a;
    const KernelThreadInfo *tb = (const KernelThreadInfo *)b;
    long total_a = ta->voluntary_ctxt + ta->nonvoluntary_ctxt;
    long total_b = tb->voluntary_ctxt + tb->nonvoluntary_ctxt;
    if (total_b > total_a) return 1;
    if (total_b < total_a) return -1;
    return 0;
}

int scan_kernel_threads(KernelThreadInfo *threads, int max_threads, int *out_count) {
    DIR *dir = opendir("/proc");
    if (!dir) return -1;

    struct dirent *entry;
    KernelThreadInfo pool[256];
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < 256) {
        if (!isdigit(entry->d_name[0])) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;

        if (parse_kernel_thread(pid, &pool[count]) == 0) {
            count++;
        }
    }
    closedir(dir);

    if (count > 0) {
        qsort(pool, count, sizeof(KernelThreadInfo), compare_kthreads);
    }

    int selected = (count < max_threads) ? count : max_threads;
    for (int i = 0; i < selected; i++) {
        threads[i] = pool[i];
    }
    *out_count = selected;
    return 0;
}
