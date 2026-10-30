/**
 * ============================================================================
 * syscalls.h - Kernel System Calls (sysinfo & getrusage)
 * ============================================================================
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "collector.h"

int collect_sysinfo(KernelSyscallMetrics *sys);
int collect_getrusage(KernelSyscallMetrics *sys);

#endif /* SYSCALLS_H */
