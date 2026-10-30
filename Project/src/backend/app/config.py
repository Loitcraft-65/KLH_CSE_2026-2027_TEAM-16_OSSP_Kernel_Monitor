"""
Kernel Monitor - Backend Configuration & Alert Rules Config
Lead: Saahas Vaddi (Roll: 2520030256)
"""

from pydantic import BaseModel

class ThresholdConfig(BaseModel):
    # 1. Kernel CPU Scheduler
    max_kernel_cpu_pct: float = 30.0        # Ring-0 Kernel execution time (%)
    max_ctxt_rate_sec: float = 15000.0      # Scheduler Context switches / second
    
    # 2. Kernel Memory Allocator & SLAB
    max_slab_pct: float = 20.0              # Kernel SLAB cache growth (% of RAM)
    max_pgfault_rate_sec: float = 8000.0    # Minor page table faults / second
    max_pgmajfault_rate_sec: float = 50.0   # Major disk page-ins / second
    
    # 3. VFS Tables
    max_file_descriptor_pct: float = 75.0   # VFS file table limit exhaustion (%)
    max_dentry_unused: int = 50000          # Stale dentry cache accumulation

thresholds = ThresholdConfig()

SERVER_HOST = "0.0.0.0"
SERVER_PORT = 8000
DB_PATH = "logs/kernel_metrics.db"
LOG_FILE_PATH = "logs/kernel_monitor.log"
