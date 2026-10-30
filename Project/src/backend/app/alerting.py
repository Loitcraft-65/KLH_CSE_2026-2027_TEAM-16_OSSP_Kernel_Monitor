"""
Kernel Monitor - Rules Engine for Threshold-based Alerting
Lead: Saahas Vaddi & Rithwik Rapaka
"""

import time
from typing import List, Dict, Any
from .config import thresholds
from .database import record_alert

recent_active_alerts: List[Dict[str, Any]] = []

async def evaluate_snapshot(snap: dict) -> List[Dict[str, Any]]:
    triggered = []
    ts = snap.get("timestamp", int(time.time()))

    sched = snap.get("sched", {})
    mem = snap.get("memory", {})
    vfs = snap.get("vfs", {})

    # 1. Kernel Mode CPU (System Time in Ring 0)
    system_cpu = sched.get("system_pct", 0.0)
    if system_cpu > thresholds.max_kernel_cpu_pct:
        triggered.append({
            "timestamp": ts,
            "severity": "critical" if system_cpu > thresholds.max_kernel_cpu_pct * 1.5 else "warning",
            "subsystem": "Kernel CPU Scheduler",
            "metric_name": "Kernel Mode CPU (System)",
            "current_value": system_cpu,
            "threshold_value": thresholds.max_kernel_cpu_pct,
            "message": f"Excessive CPU execution in Ring-0 Kernel Space: {system_cpu:.1f}% exceeds threshold of {thresholds.max_kernel_cpu_pct:.1f}%"
        })

    # 2. Kernel Scheduler Context Switch Storm
    ctxt_rate = sched.get("ctxt_rate", 0.0)
    if ctxt_rate > thresholds.max_ctxt_rate_sec:
        triggered.append({
            "timestamp": ts,
            "severity": "warning",
            "subsystem": "Scheduler / Dispatcher",
            "metric_name": "Context Switches / sec",
            "current_value": ctxt_rate,
            "threshold_value": thresholds.max_ctxt_rate_sec,
            "message": f"Scheduler preemption churn: {ctxt_rate:.0f} ctxt/sec exceeds threshold of {thresholds.max_ctxt_rate_sec:.0f}"
        })

    # 3. Kernel SLAB Memory Allocation
    slab_pct = mem.get("slab_ratio_pct", 0.0)
    if slab_pct > thresholds.max_slab_pct:
        triggered.append({
            "timestamp": ts,
            "severity": "warning",
            "subsystem": "SLAB Allocator",
            "metric_name": "Kernel SLAB %",
            "current_value": slab_pct,
            "threshold_value": thresholds.max_slab_pct,
            "message": f"Kernel object cache consumption high: {slab_pct:.1f}% of total RAM allocated to SLAB"
        })

    # 4. Virtual Memory Minor Page Faults
    pgfault_rate = mem.get("pgfault_rate", 0.0)
    if pgfault_rate > thresholds.max_pgfault_rate_sec:
        triggered.append({
            "timestamp": ts,
            "severity": "warning",
            "subsystem": "Virtual Memory (MMU)",
            "metric_name": "Minor Page Faults / sec",
            "current_value": pgfault_rate,
            "threshold_value": thresholds.max_pgfault_rate_sec,
            "message": f"Elevated page table allocation rate: {pgfault_rate:.0f} minor faults/sec"
        })

    # 5. Major Page Faults (Synchronous Disk Page-Ins)
    pgmajfault_rate = mem.get("pgmajfault_rate", 0.0)
    if pgmajfault_rate > thresholds.max_pgmajfault_rate_sec:
        triggered.append({
            "timestamp": ts,
            "severity": "critical",
            "subsystem": "Virtual Memory / Paging Engine",
            "metric_name": "Major Page Faults / sec",
            "current_value": pgmajfault_rate,
            "threshold_value": thresholds.max_pgmajfault_rate_sec,
            "message": f"Kernel paging thrashing: {pgmajfault_rate:.0f} major faults/sec requiring disk page read"
        })

    # 6. VFS File Descriptor exhaustion
    file_alloc_pct = vfs.get("alloc_pct", 0.0)
    if file_alloc_pct > thresholds.max_file_descriptor_pct:
        triggered.append({
            "timestamp": ts,
            "severity": "critical",
            "subsystem": "VFS File Table",
            "metric_name": "File Descriptors %",
            "current_value": file_alloc_pct,
            "threshold_value": thresholds.max_file_descriptor_pct,
            "message": f"Kernel file descriptor table near limit: {file_alloc_pct:.1f}% allocated"
        })

    for alert in triggered:
        await record_alert(alert)

    global recent_active_alerts
    recent_active_alerts = triggered
    return triggered
