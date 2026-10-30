/**
 * ============================================================================
 * syscalls.c - Implementation of sysinfo() and getrusage() system calls
 * ============================================================================
 */

#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <unistd.h>
#include "../include/syscalls.h"

int collect_sysinfo(KernelSyscallMetrics *sys) {
    struct sysinfo s;
    if (sysinfo(&s) != 0) {
        return -1;
    }
    sys->uptime_seconds = s.uptime;
    sys->loads[0] = s.loads[0];
    sys->loads[1] = s.loads[1];
    sys->loads[2] = s.loads[2];
    return 0;
}

int collect_getrusage(KernelSyscallMetrics *sys) {
    struct rusage u;
    if (getrusage(RUSAGE_SELF, &u) != 0) {
        return -1;
    }
    sys->rusage_user_sec = (double)u.ru_utime.tv_sec + (double)u.ru_utime.tv_usec / 1e6;
    sys->rusage_kernel_sec = (double)u.ru_stime.tv_sec + (double)u.ru_stime.tv_usec / 1e6;
    sys->rusage_minor_faults = u.ru_minflt;
    sys->rusage_major_faults = u.ru_majflt;
    sys->rusage_voluntary_ctxt = u.ru_nvcsw;
    sys->rusage_involuntary_ctxt = u.ru_nivcsw;
    return 0;
}
