/**
 * ============================================================================
 * kthreads.h - Declarations for Linux Kernel Threads inspection (Ring-0)
 * ============================================================================
 */

#ifndef KTHREADS_H
#define KTHREADS_H

#include "collector.h"

int scan_kernel_threads(KernelThreadInfo *threads, int max_threads, int *out_count);

#endif /* KTHREADS_H */
