/**
 * ============================================================================
 * proc_parser.h - Declarations for parsing Linux Kernel /proc Subsystems
 * ============================================================================
 */

#ifndef PROC_PARSER_H
#define PROC_PARSER_H

#include "collector.h"

int parse_proc_version(char *version, size_t max_len);
int parse_proc_loadavg(double *l1, double *l5, double *l15, int *runnable, int *total);
int parse_proc_stat(KernelSchedMetrics *sched);
int parse_proc_meminfo(KernelMemoryMetrics *mem);
int parse_proc_vmstat(KernelMemoryMetrics *mem);
int parse_proc_vfs(KernelVfsMetrics *vfs);
int parse_proc_interrupts_and_softirqs(KernelInterruptMetrics *intr);
int parse_proc_tunables(KernelTunablesMetrics *tunables);

#endif /* PROC_PARSER_H */
