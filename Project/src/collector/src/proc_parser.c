/**
 * ============================================================================
 * proc_parser.c - Deep Linux Kernel Subsystem Parsers
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "../include/proc_parser.h"

static uint64_t prev_sys = 0, prev_usr = 0, prev_irq = 0, prev_softirq = 0, prev_idle = 0, prev_iowait = 0;
static uint64_t prev_ctxt = 0;
static uint64_t prev_pgfault = 0, prev_pgmajfault = 0;
static uint64_t prev_res_ipis = 0, prev_tlb = 0;
static struct timespec prev_time = {0, 0};

int parse_proc_version(char *version, size_t max_len) {
    FILE *fp = fopen("/proc/version", "r");
    if (!fp) {
        snprintf(version, max_len, "Linux Kernel (Generic)");
        return -1;
    }
    if (fgets(version, max_len, fp) != NULL) {
        size_t len = strlen(version);
        if (len > 0 && version[len - 1] == '\n') version[len - 1] = '\0';
    }
    fclose(fp);
    return 0;
}

int parse_proc_loadavg(double *l1, double *l5, double *l15, int *runnable, int *total) {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return -1;
    int last_pid;
    if (fscanf(fp, "%lf %lf %lf %d/%d %d", l1, l5, l15, runnable, total, &last_pid) != 6) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int parse_proc_stat(KernelSchedMetrics *sched) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1;

    char line[512];
    int parsed_cpu = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!parsed_cpu && strncmp(line, "cpu ", 4) == 0) {
            uint64_t nice, steal;
            sscanf(line + 4, "%lu %lu %lu %lu %lu %lu %lu %lu",
                   &sched->user_ticks, &nice, &sched->system_ticks, &sched->idle_ticks,
                   &sched->iowait_ticks, &sched->irq_ticks, &sched->softirq_ticks, &steal);
            sched->user_ticks += nice;
            parsed_cpu = 1;
        } else if (strncmp(line, "ctxt ", 5) == 0) {
            sscanf(line + 5, "%lu", &sched->ctxt_switches);
        } else if (strncmp(line, "processes ", 10) == 0) {
            sscanf(line + 10, "%lu", &sched->processes_forked);
        } else if (strncmp(line, "procs_running ", 14) == 0) {
            sscanf(line + 14, "%u", &sched->procs_running);
        } else if (strncmp(line, "procs_blocked ", 14) == 0) {
            sscanf(line + 14, "%u", &sched->procs_blocked);
        }
    }
    fclose(fp);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double dt = 1.0;
    if (prev_time.tv_sec != 0) {
        dt = (now.tv_sec - prev_time.tv_sec) + (now.tv_nsec - prev_time.tv_nsec) / 1e9;
        if (dt <= 0.0001) dt = 1.0;
    }

    uint64_t d_sys = (sched->system_ticks >= prev_sys) ? (sched->system_ticks - prev_sys) : 0;
    uint64_t d_usr = (sched->user_ticks >= prev_usr) ? (sched->user_ticks - prev_usr) : 0;
    uint64_t d_irq = (sched->irq_ticks >= prev_irq) ? (sched->irq_ticks - prev_irq) : 0;
    uint64_t d_sirq = (sched->softirq_ticks >= prev_softirq) ? (sched->softirq_ticks - prev_softirq) : 0;
    uint64_t d_wait = (sched->iowait_ticks >= prev_iowait) ? (sched->iowait_ticks - prev_iowait) : 0;
    uint64_t d_idle = (sched->idle_ticks >= prev_idle) ? (sched->idle_ticks - prev_idle) : 0;

    uint64_t total = d_sys + d_usr + d_irq + d_sirq + d_wait + d_idle;
    if (total > 0 && prev_sys != 0) {
        sched->system_pct = (double)d_sys * 100.0 / total;
        sched->user_pct = (double)d_usr * 100.0 / total;
        sched->irq_pct = (double)d_irq * 100.0 / total;
        sched->softirq_pct = (double)d_sirq * 100.0 / total;
        sched->iowait_pct = (double)d_wait * 100.0 / total;
    } else {
        sched->system_pct = 0.0;
        sched->user_pct = 0.0;
        sched->irq_pct = 0.0;
        sched->softirq_pct = 0.0;
        sched->iowait_pct = 0.0;
    }

    if (prev_ctxt != 0 && sched->ctxt_switches >= prev_ctxt) {
        sched->ctxt_rate_sec = (double)(sched->ctxt_switches - prev_ctxt) / dt;
    } else {
        sched->ctxt_rate_sec = 0.0;
    }

    prev_sys = sched->system_ticks;
    prev_usr = sched->user_ticks;
    prev_irq = sched->irq_ticks;
    prev_softirq = sched->softirq_ticks;
    prev_iowait = sched->iowait_ticks;
    prev_idle = sched->idle_ticks;
    prev_ctxt = sched->ctxt_switches;
    prev_time = now;

    return 0;
}

int parse_proc_meminfo(KernelMemoryMetrics *mem) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    char key[64];
    uint64_t val;
    char unit[16];

    while (fscanf(fp, "%63s %lu %15s", key, &val, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0) mem->mem_total_kb = val;
        else if (strcmp(key, "MemFree:") == 0) mem->mem_free_kb = val;
        else if (strcmp(key, "Slab:") == 0) mem->slab_total_kb = val;
        else if (strcmp(key, "SReclaimable:") == 0) mem->slab_reclaim_kb = val;
        else if (strcmp(key, "SUnreclaim:") == 0) mem->slab_unreclaim_kb = val;
        else if (strcmp(key, "PageTables:") == 0) mem->page_tables_kb = val;
        else if (strcmp(key, "KernelStack:") == 0) mem->kernel_stack_kb = val;
        else if (strcmp(key, "VmallocUsed:") == 0) mem->vmalloc_used_kb = val;
        else if (strcmp(key, "Dirty:") == 0) mem->dirty_kb = val;
        else if (strcmp(key, "Writeback:") == 0) mem->writeback_kb = val;
    }
    fclose(fp);

    if (mem->mem_total_kb > 0) {
        mem->slab_ratio_pct = (double)mem->slab_total_kb * 100.0 / mem->mem_total_kb;
    }
    return 0;
}

int parse_proc_vmstat(KernelMemoryMetrics *mem) {
    FILE *fp = fopen("/proc/vmstat", "r");
    if (!fp) return -1;

    char key[64];
    uint64_t val;

    while (fscanf(fp, "%63s %lu", key, &val) == 2) {
        if (strcmp(key, "pgfault") == 0) mem->pgfault = val;
        else if (strcmp(key, "pgmajfault") == 0) mem->pgmajfault = val;
    }
    fclose(fp);

    if (prev_pgfault != 0 && mem->pgfault >= prev_pgfault) {
        mem->pgfault_rate_sec = (double)(mem->pgfault - prev_pgfault);
    } else {
        mem->pgfault_rate_sec = 0.0;
    }

    if (prev_pgmajfault != 0 && mem->pgmajfault >= prev_pgmajfault) {
        mem->pgmajfault_rate_sec = (double)(mem->pgmajfault - prev_pgmajfault);
    } else {
        mem->pgmajfault_rate_sec = 0.0;
    }

    prev_pgfault = mem->pgfault;
    prev_pgmajfault = mem->pgmajfault;
    return 0;
}

int parse_proc_vfs(KernelVfsMetrics *vfs) {
    // 1. /proc/sys/fs/file-nr
    FILE *fp = fopen("/proc/sys/fs/file-nr", "r");
    if (fp) {
        uint64_t unused;
        if (fscanf(fp, "%lu %lu %lu", &vfs->allocated_files, &unused, &vfs->max_files) == 3) {
            if (vfs->max_files > 0) {
                vfs->file_alloc_pct = (double)vfs->allocated_files * 100.0 / vfs->max_files;
            }
        }
        fclose(fp);
    }

    // 2. /proc/sys/fs/dentry-state (nr_dentry, nr_unused, age_limit, want_pages, dummy, dummy)
    fp = fopen("/proc/sys/fs/dentry-state", "r");
    if (fp) {
        fscanf(fp, "%lu %lu", &vfs->dentry_count, &vfs->dentry_unused);
        fclose(fp);
    }

    // 3. /proc/sys/fs/inode-nr (nr_inodes, nr_free_inodes)
    fp = fopen("/proc/sys/fs/inode-nr", "r");
    if (fp) {
        uint64_t free_inodes;
        fscanf(fp, "%lu %lu", &vfs->inode_count, &free_inodes);
        fclose(fp);
    }
    return 0;
}

static uint64_t sum_line_values(const char *line) {
    uint64_t sum = 0;
    char temp[1024];
    strncpy(temp, line, sizeof(temp) - 1);
    char *token = strtok(temp, " \t\n");
    while (token) {
        char *endptr;
        unsigned long val = strtoul(token, &endptr, 10);
        if (endptr != token && *endptr == '\0') {
            sum += val;
        }
        token = strtok(NULL, " \t\n");
    }
    return sum;
}

int parse_proc_interrupts_and_softirqs(KernelInterruptMetrics *intr) {
    // 1. /proc/interrupts for RES (Rescheduling), CAL (Function Call), TLB (TLB Shootdowns)
    FILE *fp = fopen("/proc/interrupts", "r");
    if (fp) {
        char line[2048];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, " RES:", 5) == 0 || strncmp(line, "RES:", 4) == 0) {
                intr->res_ipis = sum_line_values(line + 4);
            } else if (strncmp(line, " CAL:", 5) == 0 || strncmp(line, "CAL:", 4) == 0) {
                intr->cal_ipis = sum_line_values(line + 4);
            } else if (strncmp(line, " TLB:", 5) == 0 || strncmp(line, "TLB:", 4) == 0) {
                intr->tlb_shootdowns = sum_line_values(line + 4);
            }
        }
        fclose(fp);
    }

    // Rates
    if (prev_res_ipis != 0 && intr->res_ipis >= prev_res_ipis) {
        intr->res_ipis_rate_sec = (double)(intr->res_ipis - prev_res_ipis);
    } else {
        intr->res_ipis_rate_sec = 0.0;
    }

    if (prev_tlb != 0 && intr->tlb_shootdowns >= prev_tlb) {
        intr->tlb_rate_sec = (double)(intr->tlb_shootdowns - prev_tlb);
    } else {
        intr->tlb_rate_sec = 0.0;
    }

    prev_res_ipis = intr->res_ipis;
    prev_tlb = intr->tlb_shootdowns;

    // 2. /proc/softirqs for SCHED, RCU, TIMER, NET_RX
    fp = fopen("/proc/softirqs", "r");
    if (fp) {
        char line[2048];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "SCHED:")) {
                intr->softirq_sched = sum_line_values(line + 6);
            } else if (strstr(line, "RCU:")) {
                intr->softirq_rcu = sum_line_values(line + 4);
            } else if (strstr(line, "TIMER:")) {
                intr->softirq_timer = sum_line_values(line + 6);
            } else if (strstr(line, "NET_RX:")) {
                intr->softirq_net_rx = sum_line_values(line + 7);
            }
        }
        fclose(fp);
    }
    return 0;
}

int parse_proc_tunables(KernelTunablesMetrics *tunables) {
    FILE *fp = fopen("/proc/sys/vm/swappiness", "r");
    if (fp) {
        fscanf(fp, "%d", &tunables->swappiness);
        fclose(fp);
    }
    fp = fopen("/proc/sys/vm/vfs_cache_pressure", "r");
    if (fp) {
        fscanf(fp, "%d", &tunables->vfs_cache_pressure);
        fclose(fp);
    }
    fp = fopen("/proc/sys/vm/dirty_ratio", "r");
    if (fp) {
        fscanf(fp, "%d", &tunables->dirty_ratio);
        fclose(fp);
    }
    fp = fopen("/proc/sys/kernel/pid_max", "r");
    if (fp) {
        fscanf(fp, "%ld", &tunables->pid_max);
        fclose(fp);
    }
    return 0;
}
