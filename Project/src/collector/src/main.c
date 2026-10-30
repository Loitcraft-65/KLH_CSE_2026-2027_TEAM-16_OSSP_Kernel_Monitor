/**
 * ============================================================================
 * Kernel Monitor - C Main Entry Point (Subsystem Telemetry Engine)
 * Project: Operating Systems and Systems Programming (25CS2104E)
 * Team: Lohith Sanapala, Saahas Vaddi, Rithwik Rapaka (Team 16)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "../include/collector.h"
#include "../include/proc_parser.h"
#include "../include/syscalls.h"
#include "../include/kthreads.h"

static volatile int keep_running = 1;

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

int init_collector(void) {
    return 0;
}

int collect_kernel_telemetry(KernelTelemetrySnapshot *snap) {
    memset(snap, 0, sizeof(KernelTelemetrySnapshot));
    snap->timestamp = time(NULL);

    parse_proc_version(snap->kernel_version, sizeof(snap->kernel_version));
    parse_proc_loadavg(&snap->load_1m, &snap->load_5m, &snap->load_15m,
                       &snap->runnable_threads, &snap->total_scheduling_entities);
    parse_proc_stat(&snap->sched);
    parse_proc_meminfo(&snap->mem);
    parse_proc_vmstat(&snap->mem);
    parse_proc_vfs(&snap->vfs);
    parse_proc_interrupts_and_softirqs(&snap->intr);
    parse_proc_tunables(&snap->tunables);
    collect_sysinfo(&snap->sys);
    collect_getrusage(&snap->sys);
    scan_kernel_threads(snap->kthreads, MAX_KTHREADS, &snap->kthread_count);

    return 0;
}

void print_terminal_dashboard(const KernelTelemetrySnapshot *snap) {
    printf("\033[H\033[J");

    printf("\033[1;36m====================================================================================\033[0m\n");
    printf("\033[1;37m   LINUX KERNEL SUBSYSTEM MONITOR (OSSP 25CS2104E) - RING-0 TELEMETRY                \033[0m\n");
    printf("\033[1;34m   Kernel: %-42s  Uptime: %ld s\033[0m\n", snap->kernel_version, snap->sys.uptime_seconds);
    printf("\033[1;36m====================================================================================\033[0m\n\n");

    // 1. Kernel CPU Scheduler & Execution State
    printf("\033[1;33m[1. KERNEL CPU SCHEDULER & DISPATCH]\033[0m\n");
    printf("  Execution Mode: Kernel Space (Ring-0): \033[1;31m%5.1f%%\033[0m | User Space (Ring-3): \033[32m%5.1f%%\033[0m | I/O Wait: \033[33m%4.1f%%\033[0m\n",
           snap->sched.system_pct, snap->sched.user_pct, snap->sched.iowait_pct);
    printf("  Context Switches: \033[1;35m%lu\033[0m total (\033[1m%.0f / sec\033[0m) | Forked Processes: %lu\n",
           snap->sched.ctxt_switches, snap->sched.ctxt_rate_sec, snap->sched.processes_forked);
    printf("  CFS Runqueue: %d runnable / %d total entities | Blocked in D-state: %u\n\n",
           snap->runnable_threads, snap->total_scheduling_entities, snap->sched.procs_blocked);

    // 2. Kernel Memory & SLAB Allocator
    printf("\033[1;33m[2. KERNEL MEMORY & SLAB ALLOCATOR]\033[0m\n");
    printf("  SLAB Cache: \033[1;32m%lu MB\033[0m (Reclaimable: %lu MB | Unreclaimable: %lu MB)\n",
           snap->mem.slab_total_kb / 1024, snap->mem.slab_reclaim_kb / 1024, snap->mem.slab_unreclaim_kb / 1024);
    printf("  Kernel Structures: PageTables: %lu MB | KernelStacks: %lu MB | VmallocUsed: %lu MB\n",
           snap->mem.page_tables_kb / 1024, snap->mem.kernel_stack_kb / 1024, snap->mem.vmalloc_used_kb / 1024);
    printf("  Dirty Cache: %lu MB | Writeback: %lu MB | Minor Faults: %.0f /s | Major Faults: \033[1;31m%.0f /s\033[0m\n\n",
           snap->mem.dirty_kb / 1024, snap->mem.writeback_kb / 1024, snap->mem.pgfault_rate_sec, snap->mem.pgmajfault_rate_sec);

    // 3. VFS Kernel Tables & Inode Cache
    printf("\033[1;33m[3. VFS KERNEL CACHES & TABLES]\033[0m\n");
    printf("  Dentry Cache (/proc/sys/fs/dentry-state): \033[1;36m%lu\033[0m cached (%lu unused)\n",
           snap->vfs.dentry_count, snap->vfs.dentry_unused);
    printf("  Inode Table  (/proc/sys/fs/inode-nr):     \033[1;36m%lu\033[0m active inodes\n",
           snap->vfs.inode_count);
    printf("  File Handles (/proc/sys/fs/file-nr):      %lu allocated / %lu max (%.2f%% used)\n\n",
           snap->vfs.allocated_files, snap->vfs.max_files, snap->vfs.file_alloc_pct);

    // 4. Interrupts, IPIs & SoftIRQs
    printf("\033[1;33m[4. INTERRUPTS, IPIs & SOFTIRQs]\033[0m\n");
    printf("  Scheduler IPIs (RES): \033[1m%lu\033[0m (%.0f /s) | Function Calls (CAL): %lu | TLB Shootdowns: %lu\n",
           snap->intr.res_ipis, snap->intr.res_ipis_rate_sec, snap->intr.cal_ipis, snap->intr.tlb_shootdowns);
    printf("  SoftIRQs: SCHED: %lu | RCU: %lu | TIMER: %lu | NET_RX: %lu\n\n",
           snap->intr.softirq_sched, snap->intr.softirq_rcu, snap->intr.softirq_timer, snap->intr.softirq_net_rx);

    // 5. Kernel Tunables (sysctl)
    printf("\033[1;33m[5. KERNEL SYSCTL TUNABLES]\033[0m\n");
    printf("  vm.vfs_cache_pressure: \033[1m%d\033[0m | vm.swappiness: %d | vm.dirty_ratio: %d%% | kernel.pid_max: %ld\n\n",
           snap->tunables.vfs_cache_pressure, snap->tunables.swappiness, snap->tunables.dirty_ratio, snap->tunables.pid_max);

    // 6. Ring-0 Kernel Threads
    printf("\033[1;33m[6. RING-0 KERNEL THREADS (kthreads)]\033[0m\n");
    printf("  %-7s %-6s %-24s %-14s %-14s %-6s\n",
           "PID", "PPID", "KERNEL THREAD", "VOLUNTARY_YIELD", "SCHED_PREEMPT", "STATE");
    printf("  -----------------------------------------------------------------------------\n");
    int count = (snap->kthread_count < 8) ? snap->kthread_count : 8;
    for (int i = 0; i < count; i++) {
        const KernelThreadInfo *kt = &snap->kthreads[i];
        printf("  %-7d %-6d %-24s %-14ld %-14ld %c\n",
               kt->pid, kt->ppid, kt->name, kt->voluntary_ctxt, kt->nonvoluntary_ctxt, kt->state);
    }
    printf("\n\033[90m(Refreshes every 1s. Press Ctrl+C to exit)\033[0m\n");
}

int format_telemetry_json(const KernelTelemetrySnapshot *snap, char *buf, size_t buf_size) {
    int offset = snprintf(buf, buf_size,
        "{"
        "\"timestamp\":%ld,"
        "\"kernel_version\":\"%s\","
        "\"uptime\":%ld,"
        "\"sched\":{\"system_pct\":%.2f,\"user_pct\":%.2f,\"irq_pct\":%.2f,\"softirq_pct\":%.2f,\"iowait_pct\":%.2f,\"ctxt_rate\":%.2f,\"ctxt_total\":%lu,\"procs_running\":%u,\"procs_blocked\":%u,\"processes_forked\":%lu,\"load_1m\":%.2f,\"load_5m\":%.2f,\"load_15m\":%.2f,\"runnable\":%d,\"total_entities\":%d},"
        "\"memory\":{\"slab_total_kb\":%lu,\"slab_reclaim_kb\":%lu,\"slab_unreclaim_kb\":%lu,\"page_tables_kb\":%lu,\"kernel_stack_kb\":%lu,\"vmalloc_used_kb\":%lu,\"dirty_kb\":%lu,\"writeback_kb\":%lu,\"slab_ratio_pct\":%.2f,\"pgfault_rate\":%.2f,\"pgmajfault_rate\":%.2f,\"mem_total_kb\":%lu,\"mem_free_kb\":%lu},"
        "\"vfs\":{\"allocated_files\":%lu,\"max_files\":%lu,\"dentry_count\":%lu,\"dentry_unused\":%lu,\"inode_count\":%lu,\"alloc_pct\":%.2f},"
        "\"intr\":{\"res_ipis\":%lu,\"res_ipis_rate\":%.2f,\"cal_ipis\":%lu,\"tlb_shootdowns\":%lu,\"tlb_rate\":%.2f,\"softirq_sched\":%lu,\"softirq_rcu\":%lu,\"softirq_timer\":%lu,\"softirq_net_rx\":%lu},"
        "\"tunables\":{\"swappiness\":%d,\"vfs_cache_pressure\":%d,\"dirty_ratio\":%d,\"pid_max\":%ld},"
        "\"syscalls\":{\"rusage_user_s\":%.4f,\"rusage_kernel_s\":%.4f,\"minor_faults\":%ld,\"major_faults\":%ld,\"vol_ctxt\":%ld,\"invol_ctxt\":%ld},"
        "\"kthreads\":[",
        snap->timestamp,
        snap->kernel_version,
        snap->sys.uptime_seconds,
        snap->sched.system_pct, snap->sched.user_pct, snap->sched.irq_pct, snap->sched.softirq_pct, snap->sched.iowait_pct, snap->sched.ctxt_rate_sec, snap->sched.ctxt_switches, snap->sched.procs_running, snap->sched.procs_blocked, snap->sched.processes_forked, snap->load_1m, snap->load_5m, snap->load_15m, snap->runnable_threads, snap->total_scheduling_entities,
        snap->mem.slab_total_kb, snap->mem.slab_reclaim_kb, snap->mem.slab_unreclaim_kb, snap->mem.page_tables_kb, snap->mem.kernel_stack_kb, snap->mem.vmalloc_used_kb, snap->mem.dirty_kb, snap->mem.writeback_kb, snap->mem.slab_ratio_pct, snap->mem.pgfault_rate_sec, snap->mem.pgmajfault_rate_sec, snap->mem.mem_total_kb, snap->mem.mem_free_kb,
        snap->vfs.allocated_files, snap->vfs.max_files, snap->vfs.dentry_count, snap->vfs.dentry_unused, snap->vfs.inode_count, snap->vfs.file_alloc_pct,
        snap->intr.res_ipis, snap->intr.res_ipis_rate_sec, snap->intr.cal_ipis, snap->intr.tlb_shootdowns, snap->intr.tlb_rate_sec, snap->intr.softirq_sched, snap->intr.softirq_rcu, snap->intr.softirq_timer, snap->intr.softirq_net_rx,
        snap->tunables.swappiness, snap->tunables.vfs_cache_pressure, snap->tunables.dirty_ratio, snap->tunables.pid_max,
        snap->sys.rusage_user_sec, snap->sys.rusage_kernel_sec, snap->sys.rusage_minor_faults, snap->sys.rusage_major_faults, snap->sys.rusage_voluntary_ctxt, snap->sys.rusage_involuntary_ctxt
    );

    int count = (snap->kthread_count < 16) ? snap->kthread_count : 16;
    for (int i = 0; i < count; i++) {
        const KernelThreadInfo *kt = &snap->kthreads[i];
        int written = snprintf(buf + offset, buf_size - offset,
            "%s{\"pid\":%d,\"ppid\":%d,\"name\":\"%s\",\"vol_ctxt\":%ld,\"invol_ctxt\":%ld,\"state\":\"%c\"}",
            (i > 0) ? "," : "",
            kt->pid, kt->ppid, kt->name,
            kt->voluntary_ctxt, kt->nonvoluntary_ctxt, kt->state);
        offset += written;
        if (offset >= (int)buf_size - 10) break;
    }

    if (offset < (int)buf_size - 3) {
        strcat(buf, "]}");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int mode_json = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0) {
            mode_json = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("  --terminal    Display live kernel telemetry dashboard (default)\n");
            printf("  --json        Output real-time JSON stream to stdout\n");
            return 0;
        }
    }

    init_collector();
    KernelTelemetrySnapshot snap;
    char json_buffer[JSON_BUF_SIZE];

    // Seed initial calibration delta
    collect_kernel_telemetry(&snap);
    usleep(250000);

    while (keep_running) {
        collect_kernel_telemetry(&snap);

        if (mode_json) {
            format_telemetry_json(&snap, json_buffer, sizeof(json_buffer));
            printf("%s\n", json_buffer);
            fflush(stdout);
        } else {
            print_terminal_dashboard(&snap);
        }

        sleep(1);
    }

    printf("\nKernel Monitor Collector gracefully terminated.\n");
    return 0;
}
