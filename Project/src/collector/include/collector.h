/**
 * ============================================================================
 * Kernel Monitor - C Data Collection Module
 * Project: Operating Systems and Systems Programming (25CS2104E)
 * Team: Lohith Sanapala (Roll: 2520030023) - Kernel Collection Module
 * ============================================================================
 * collector.h - Deep Linux Kernel Subsystems & Ring-0 Internal Telemetry
 */

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdint.h>
#include <time.h>

#define MAX_KTHREADS 32
#define MAX_NAME_LEN 64
#define JSON_BUF_SIZE 16384

/* 1. Kernel CPU Scheduler & Execution State (/proc/stat) */
typedef struct {
    uint64_t system_ticks;     // Ring-0 Kernel mode execution time
    uint64_t user_ticks;       // Ring-3 User mode execution time
    uint64_t irq_ticks;        // Hardware Interrupt servicing
    uint64_t softirq_ticks;    // Software Interrupt (ksoftirqd) servicing
    uint64_t iowait_ticks;     // Kernel I/O wait queue stall time
    uint64_t idle_ticks;
    
    uint64_t ctxt_switches;    // Total context switches executed by kernel scheduler
    uint64_t processes_forked; // Total process creation forks by kernel
    uint32_t procs_running;    // Current runnable tasks in CFS runqueue
    uint32_t procs_blocked;    // Tasks blocked in kernel uninterruptible sleep (D)

    double system_pct;         // % CPU in Kernel Space
    double user_pct;           // % CPU in User Space
    double irq_pct;
    double softirq_pct;
    double iowait_pct;
    double ctxt_rate_sec;      // Kernel Context Switches / second
} KernelSchedMetrics;

/* 2. Kernel Memory Allocator & Virtual Memory Subsystem (/proc/meminfo, /proc/vmstat) */
typedef struct {
    uint64_t slab_total_kb;        // Total Kernel SLAB cache
    uint64_t slab_reclaim_kb;      // Reclaimable SLAB objects (dentries, inodes)
    uint64_t slab_unreclaim_kb;    // In-use non-reclaimable kernel allocations
    uint64_t page_tables_kb;       // Memory consumed by MMU Page Table structures
    uint64_t kernel_stack_kb;      // Memory allocated to Thread Kernel Stacks
    uint64_t vmalloc_used_kb;      // Virtual kernel allocations
    uint64_t dirty_kb;             // Dirty pages awaiting kernel pdflush/flusher
    uint64_t writeback_kb;         // Pages actively writing to disk
    uint64_t mem_total_kb;
    uint64_t mem_free_kb;

    uint64_t pgfault;              // Minor page faults (RAM soft-faults)
    uint64_t pgmajfault;           // Major page faults (Disk page-ins)
    double pgfault_rate_sec;
    double pgmajfault_rate_sec;
    double slab_ratio_pct;         // SLAB as % of total RAM
} KernelMemoryMetrics;

/* 3. VFS (Virtual File System) Kernel Tables (/proc/sys/fs/*) */
typedef struct {
    uint64_t allocated_files;      // Active kernel file descriptors
    uint64_t max_files;            // fs.file-max system-wide limit
    uint64_t dentry_count;         // Cached directory entry structures in VFS
    uint64_t dentry_unused;        // Unused cached dentries
    uint64_t inode_count;          // Active VFS inode structures
    double file_alloc_pct;
} KernelVfsMetrics;

/* 4. Kernel Interrupts & Traps (/proc/interrupts & /proc/softirqs) */
typedef struct {
    uint64_t res_ipis;             // Rescheduling Interrupts (IPIs across cores)
    uint64_t cal_ipis;             // Function Call Interrupts (SMP IPIs)
    uint64_t tlb_shootdowns;       // TLB shootdown flushes (MMU invalidation)
    uint64_t softirq_sched;        // Scheduler softirq executions
    uint64_t softirq_rcu;          // RCU (Read-Copy Update) softirq executions
    uint64_t softirq_timer;        // Timer softirq executions
    uint64_t softirq_net_rx;       // Network RX softirq executions
    double res_ipis_rate_sec;
    double tlb_rate_sec;
} KernelInterruptMetrics;

/* 5. Kernel Sysctl Tunables (/proc/sys/vm/* & /proc/sys/kernel/*) */
typedef struct {
    int swappiness;                // vm.swappiness (0-100)
    int vfs_cache_pressure;        // vm.vfs_cache_pressure
    int dirty_ratio;               // vm.dirty_ratio
    long pid_max;                  // kernel.pid_max
} KernelTunablesMetrics;

/* 6. Ring-0 Kernel Threads Only (kworker, ksoftirqd, rcu_preempt, migration) */
typedef struct {
    int pid;
    int ppid;
    char name[MAX_NAME_LEN];
    char state;
    long voluntary_ctxt;           // Thread voluntarily surrendered CPU
    long nonvoluntary_ctxt;        // Thread preempted by kernel scheduler
} KernelThreadInfo;

/* 7. Linux System Calls (sysinfo & getrusage) */
typedef struct {
    long uptime_seconds;
    unsigned long loads[3];
    double rusage_user_sec;
    double rusage_kernel_sec;      // Time spent executing inside kernel syscalls
    long rusage_minor_faults;
    long rusage_major_faults;
    long rusage_voluntary_ctxt;
    long rusage_involuntary_ctxt;
} KernelSyscallMetrics;

/* Complete Kernel Snapshot */
typedef struct {
    char kernel_version[128];
    time_t timestamp;

    double load_1m;
    double load_5m;
    double load_15m;
    int runnable_threads;
    int total_scheduling_entities;

    KernelSchedMetrics sched;
    KernelMemoryMetrics mem;
    KernelVfsMetrics vfs;
    KernelInterruptMetrics intr;
    KernelTunablesMetrics tunables;
    KernelSyscallMetrics sys;

    int kthread_count;
    KernelThreadInfo kthreads[MAX_KTHREADS];
} KernelTelemetrySnapshot;

/* Function prototypes */
int init_collector(void);
int collect_kernel_telemetry(KernelTelemetrySnapshot *snap);
void print_terminal_dashboard(const KernelTelemetrySnapshot *snap);
int format_telemetry_json(const KernelTelemetrySnapshot *snap, char *buffer, size_t buf_size);

#endif /* COLLECTOR_H */
