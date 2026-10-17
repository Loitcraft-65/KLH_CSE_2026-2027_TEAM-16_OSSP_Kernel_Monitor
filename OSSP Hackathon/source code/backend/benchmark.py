import asyncio
import time
from typing import List, Dict, Any
from .c_bridge import run_c_search_stream

async def run_benchmark_suite(
    directory: str,
    pattern: str = "main",
    thread_counts: List[int] = None,
    search_name: bool = True,
    search_content: bool = True
) -> Dict[str, Any]:
    if thread_counts is None:
        thread_counts = [1, 2, 4, 8, 16]

    results = []
    base_duration = None

    for threads in thread_counts:
        start_time = time.time()
        last_done = None
        total_files = 0
        total_matches = 0

        async for event in run_c_search_stream(
            directory=directory,
            pattern=pattern,
            num_threads=threads,
            search_name=search_name,
            search_content=search_content
        ):
            if event.get("type") == "done":
                last_done = event
                total_files = event.get("total_files", 0)
                total_matches = event.get("total_matches", 0)

        elapsed_ms = (time.time() - start_time) * 1000.0
        if last_done and "duration_ms" in last_done:
            # Use high precision C internal timer if available
            c_duration = last_done["duration_ms"]
            # If search was too fast, guard against 0ms division
            duration_ms = max(c_duration, 0.5)
        else:
            duration_ms = max(elapsed_ms, 0.5)

        if base_duration is None:
            base_duration = duration_ms

        speedup = round(base_duration / duration_ms, 2)
        efficiency = round((speedup / threads) * 100.0, 1) if threads > 0 else 100.0
        
        # Amdahl's Law theoretical speedup with P = 0.88 (parallel portion)
        p = 0.88
        amdahl_speedup = round(1.0 / ((1.0 - p) + (p / threads)), 2)

        throughput = round((total_files / (duration_ms / 1000.0)), 1) if duration_ms > 0 else 0

        results.append({
            "threads": threads,
            "duration_ms": round(duration_ms, 2),
            "speedup": speedup,
            "efficiency_percent": efficiency,
            "amdahl_speedup": amdahl_speedup,
            "throughput_files_sec": throughput,
            "total_files": total_files,
            "total_matches": total_matches
        })

    return {
        "directory": directory,
        "pattern": pattern,
        "baseline_single_thread_ms": base_duration,
        "results": results
    }
