"""
Kernel Monitor - Bridge between C Collector and Python Backend
Lead: Saahas Vaddi (Roll: 2520030256) & Lohith Sanapala (Roll: 2520030023)
"""

import asyncio
import json
import os
import platform
import subprocess
import threading
import time
from typing import Dict, Any, List
from .database import record_metrics
from .alerting import evaluate_snapshot

latest_snapshot: Dict[str, Any] = {}
snapshot_listeners: List[asyncio.Queue] = []
is_running = False
main_loop: asyncio.AbstractEventLoop = None

def get_latest_snapshot() -> dict:
    global latest_snapshot
    if not latest_snapshot:
        latest_snapshot = get_fallback_snapshot()
    return latest_snapshot

import math
import random

def get_fallback_snapshot() -> dict:
    """Fallback generator for pure Windows or initial startup with realistic live telemetry."""
    t = time.time()
    # Realistic organic wave variations
    wave1 = math.sin(t / 6.0)
    wave2 = math.cos(t / 4.2)
    jitter = random.uniform(-0.4, 0.4)
    
    sys_cpu = round(max(2.1, min(35.0, 7.5 + wave1 * 4.2 + jitter)), 1)
    user_cpu = round(max(4.0, min(60.0, 14.2 + wave2 * 6.5 + random.uniform(-1.0, 1.0))), 1)
    iowait = round(max(0.1, min(15.0, 1.2 + math.sin(t / 8.0) * 0.8 + random.uniform(-0.2, 0.2))), 1)
    
    base_ctxt = 4500.0 + wave1 * 1200.0 + random.uniform(-150.0, 150.0)
    ctxt_rate = round(max(1500.0, base_ctxt), 0)
    
    slab_mb = round(448 + wave2 * 18 + random.uniform(-3, 3), 1)
    slab_rec_mb = round(slab_mb * 0.44, 1)
    slab_unrec_mb = round(slab_mb - slab_rec_mb, 1)
    
    pgfault_rate = round(max(800.0, 2100.0 + wave1 * 600.0 + random.uniform(-100, 100)), 0)
    pgmajfault_rate = round(max(0.0, 1.0 + (1.0 if random.random() > 0.85 else 0.0)), 0)
    
    res_ipis = int(4200 + t % 1000 * 8 + random.randint(0, 15))
    cal_ipis = int(89400 + t % 1000 * 42 + random.randint(0, 50))
    dentry_count = int(24800 + wave1 * 300 + random.randint(-20, 20))
    inode_count = int(19600 + wave2 * 200 + random.randint(-15, 15))
    
    sched_softirq = int(64200 + t % 1000 * 18)
    rcu_softirq = int(38100 + t % 1000 * 12)
    timer_softirq = int(42000 + t % 1000 * 24)
    net_softirq = int(8400 + t % 1000 * 6)

    return {
        "timestamp": int(t),
        "kernel_version": f"Linux 6.18.33 (Ring-0 Kernel Telemetry Simulator - {platform.system()})",
        "uptime": int(t % 86400),
        "sched": {
            "system_pct": sys_cpu,
            "user_pct": user_cpu,
            "irq_pct": 0.4,
            "softirq_pct": 0.8,
            "iowait_pct": iowait,
            "ctxt_rate": ctxt_rate,
            "ctxt_total": 782910 + int(t % 10000 * 4000),
            "procs_running": 2 + (1 if wave1 > 0.5 else 0),
            "procs_blocked": 0,
            "processes_forked": 1420 + int(t % 1000),
            "load_1m": round(0.42 + wave1 * 0.15, 2),
            "load_5m": round(0.35 + wave2 * 0.08, 2),
            "load_15m": 0.22,
            "runnable": 2 + (1 if wave1 > 0.3 else 0),
            "total_entities": 284
        },
        "memory": {
            "slab_total_kb": int(slab_mb * 1024),
            "slab_reclaim_kb": int(slab_rec_mb * 1024),
            "slab_unreclaim_kb": int(slab_unrec_mb * 1024),
            "page_tables_kb": 32768 + int(wave1 * 1024),
            "kernel_stack_kb": 16384,
            "vmalloc_used_kb": 42180,
            "dirty_kb": 8192 + int(wave2 * 2048),
            "writeback_kb": 0,
            "slab_ratio_pct": round((slab_mb * 1024 / 8388608) * 100, 2),
            "pgfault_rate": pgfault_rate,
            "pgmajfault_rate": pgmajfault_rate,
            "mem_total_kb": 8388608,
            "mem_free_kb": int(5242880 + wave1 * 100000)
        },
        "vfs": {
            "allocated_files": 2840 + int(wave2 * 40),
            "max_files": 9223372036854775807,
            "dentry_count": dentry_count,
            "dentry_unused": 16200,
            "inode_count": inode_count,
            "alloc_pct": 0.001
        },
        "intr": {
            "res_ipis": res_ipis,
            "res_ipis_rate": round(110.0 + wave1 * 30.0, 1),
            "cal_ipis": cal_ipis,
            "tlb_shootdowns": 12 + int(wave1 * 3),
            "tlb_rate": 0.2,
            "softirq_sched": sched_softirq,
            "softirq_rcu": rcu_softirq,
            "softirq_timer": timer_softirq,
            "softirq_net_rx": net_softirq
        },
        "tunables": {
            "swappiness": 60,
            "vfs_cache_pressure": 100,
            "dirty_ratio": 20,
            "pid_max": 4194304
        },
        "syscalls": {
            "rusage_user_s": round(0.024 + (t % 100) * 0.001, 3),
            "rusage_kernel_s": round(0.048 + (t % 100) * 0.002, 3),
            "minor_faults": int(pgfault_rate / 10),
            "major_faults": int(pgmajfault_rate),
            "vol_ctxt": int(84 + (t % 60) * 3),
            "invol_ctxt": int(6 + (t % 20))
        },
        "kthreads": [
            {"pid": 2, "ppid": 0, "name": "kthreadd", "vol_ctxt": 1204 + int(t % 500), "invol_ctxt": 0, "state": "S"},
            {"pid": 3, "ppid": 2, "name": "rcu_gp", "vol_ctxt": 8920 + int(t % 800), "invol_ctxt": 0, "state": "S"},
            {"pid": 4, "ppid": 2, "name": "rcu_par_gp", "vol_ctxt": 412 + int(t % 200), "invol_ctxt": 0, "state": "S"},
            {"pid": 10, "ppid": 2, "name": "kworker/0:0H", "vol_ctxt": 240, "invol_ctxt": 2, "state": "S"},
            {"pid": 14, "ppid": 2, "name": "ksoftirqd/0", "vol_ctxt": 840 + int(t % 300), "invol_ctxt": 1, "state": "S"},
            {"pid": 15, "ppid": 2, "name": "ksoftirqd/1", "vol_ctxt": 790 + int(t % 300), "invol_ctxt": 0, "state": "S"},
            {"pid": 16, "ppid": 2, "name": "rcu_preempt", "vol_ctxt": 3410 + int(t % 600), "invol_ctxt": 2, "state": "S"},
            {"pid": 17, "ppid": 2, "name": "migration/0", "vol_ctxt": 11200 + int(t % 900), "invol_ctxt": 0, "state": "S"},
            {"pid": 18, "ppid": 2, "name": "migration/1", "vol_ctxt": 10840 + int(t % 900), "invol_ctxt": 0, "state": "S"},
            {"pid": 19, "ppid": 2, "name": "cpuhp/0", "vol_ctxt": 18, "invol_ctxt": 0, "state": "S"},
            {"pid": 20, "ppid": 2, "name": "cpuhp/1", "vol_ctxt": 18, "invol_ctxt": 0, "state": "S"},
            {"pid": 28, "ppid": 2, "name": "kworker/u16:0", "vol_ctxt": 1590 + int(t % 400), "invol_ctxt": 4, "state": "S"},
            {"pid": 29, "ppid": 2, "name": "kworker/u16:1", "vol_ctxt": 1420 + int(t % 400), "invol_ctxt": 3, "state": "S"},
            {"pid": 35, "ppid": 2, "name": "kcompactd0", "vol_ctxt": 88, "invol_ctxt": 0, "state": "S"},
            {"pid": 42, "ppid": 2, "name": "khugepaged", "vol_ctxt": 640 + int(t % 150), "invol_ctxt": 1, "state": "S"},
            {"pid": 60, "ppid": 2, "name": "jbd2/sda1-8", "vol_ctxt": 480 + int(t % 200), "invol_ctxt": 0, "state": "S"},
            {"pid": 80, "ppid": 2, "name": "kauditd", "vol_ctxt": 150, "invol_ctxt": 0, "state": "S"},
            {"pid": 95, "ppid": 2, "name": "oom_reaper", "vol_ctxt": 1, "invol_ctxt": 0, "state": "S"}
        ]
    }

async def handle_new_snapshot(data: dict):
    global latest_snapshot
    latest_snapshot = data

    await evaluate_snapshot(data)
    await record_metrics(data)

    for q in list(snapshot_listeners):
        try:
            await q.put(data)
        except Exception:
            pass

def _collector_worker():
    global is_running, main_loop
    workspace_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

    cmd = None
    if platform.system() == "Linux":
        c_bin = os.path.join(workspace_dir, "kernel_collector")
        if os.path.exists(c_bin):
            cmd = [c_bin, "--json"]
    elif platform.system() == "Windows":
        cmd = ["wsl", "/mnt/e/OSSP/kernel_collector", "--json"]

    while is_running:
        proc = None
        has_streamed = False
        try:
            if cmd:
                print(f"[Collector Bridge] Running: {' '.join(cmd)}")
                proc = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                    text=True,
                    bufsize=1
                )

                for line in iter(proc.stdout.readline, ''):
                    if not is_running:
                        break
                    line_str = line.strip()
                    if line_str.startswith("{") and line_str.endswith("}"):
                        try:
                            snap = json.loads(line_str)
                            has_streamed = True
                            if main_loop and main_loop.is_running():
                                asyncio.run_coroutine_threadsafe(handle_new_snapshot(snap), main_loop)
                        except json.JSONDecodeError:
                            continue
        except Exception as e:
            print(f"[Collector Bridge] Process exception: {e}")

        if proc:
            try:
                proc.terminate()
                proc.wait(timeout=1)
            except Exception:
                pass

        if not has_streamed and is_running:
            snap = get_fallback_snapshot()
            if main_loop and main_loop.is_running():
                asyncio.run_coroutine_threadsafe(handle_new_snapshot(snap), main_loop)
            time.sleep(1.0)
        else:
            time.sleep(1.0)

async def run_collector_bridge():
    global is_running, main_loop
    is_running = True
    main_loop = asyncio.get_running_loop()

    global latest_snapshot
    if not latest_snapshot:
        latest_snapshot = get_fallback_snapshot()

    thread = threading.Thread(target=_collector_worker, daemon=True)
    thread.start()

    while is_running:
        await asyncio.sleep(1.0)

def stop_collector_bridge():
    global is_running
    is_running = False
