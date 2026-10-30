"""
Kernel Monitor - SQLite Database & Historical Metrics Storage
Lead: Saahas Vaddi (Roll: 2520030256)
"""

import aiosqlite
import os
import datetime
from .config import DB_PATH, LOG_FILE_PATH

async def init_db():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    async with aiosqlite.connect(DB_PATH) as db:
        await db.execute("""
            CREATE TABLE IF NOT EXISTS metrics_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER,
                kernel_cpu_pct REAL,
                user_cpu_pct REAL,
                ctxt_rate REAL,
                slab_total_kb INTEGER,
                slab_reclaim_kb INTEGER,
                page_tables_kb INTEGER,
                kernel_stack_kb INTEGER,
                vmalloc_used_kb INTEGER,
                dentry_count INTEGER,
                inode_count INTEGER,
                res_ipis INTEGER,
                cal_ipis INTEGER,
                softirq_sched INTEGER,
                softirq_rcu INTEGER,
                pgfault_rate REAL,
                pgmajfault_rate REAL
            )
        """)
        
        await db.execute("""
            CREATE TABLE IF NOT EXISTS alerts_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER,
                severity TEXT,
                subsystem TEXT,
                metric_name TEXT,
                current_value REAL,
                threshold_value REAL,
                message TEXT
            )
        """)
        await db.commit()

async def record_metrics(m: dict):
    try:
        sched = m.get("sched", {})
        mem = m.get("memory", {})
        vfs = m.get("vfs", {})
        intr = m.get("intr", {})

        async with aiosqlite.connect(DB_PATH) as db:
            await db.execute("""
                INSERT INTO metrics_history (
                    timestamp, kernel_cpu_pct, user_cpu_pct, ctxt_rate,
                    slab_total_kb, slab_reclaim_kb, page_tables_kb, kernel_stack_kb, vmalloc_used_kb,
                    dentry_count, inode_count, res_ipis, cal_ipis, softirq_sched, softirq_rcu,
                    pgfault_rate, pgmajfault_rate
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                m.get("timestamp", 0),
                sched.get("system_pct", 0.0),
                sched.get("user_pct", 0.0),
                sched.get("ctxt_rate", 0.0),
                mem.get("slab_total_kb", 0),
                mem.get("slab_reclaim_kb", 0),
                mem.get("page_tables_kb", 0),
                mem.get("kernel_stack_kb", 0),
                mem.get("vmalloc_used_kb", 0),
                vfs.get("dentry_count", 0),
                vfs.get("inode_count", 0),
                intr.get("res_ipis", 0),
                intr.get("cal_ipis", 0),
                intr.get("softirq_sched", 0),
                intr.get("softirq_rcu", 0),
                mem.get("pgfault_rate", 0.0),
                mem.get("pgmajfault_rate", 0.0)
            ))
            await db.commit()
    except Exception as e:
        print(f"[DB Error] record_metrics: {e}")

async def record_alert(alert: dict):
    try:
        async with aiosqlite.connect(DB_PATH) as db:
            await db.execute("""
                INSERT INTO alerts_history (
                    timestamp, severity, subsystem, metric_name, current_value, threshold_value, message
                ) VALUES (?, ?, ?, ?, ?, ?, ?)
            """, (
                alert["timestamp"],
                alert["severity"],
                alert["subsystem"],
                alert["metric_name"],
                alert["current_value"],
                alert["threshold_value"],
                alert["message"]
            ))
            await db.commit()
            
        log_line = (
            f"[{datetime.datetime.fromtimestamp(alert['timestamp']).isoformat()}] "
            f"[{alert['severity'].upper()}] Subsystem: {alert['subsystem']} | "
            f"{alert['metric_name']}={alert['current_value']:.2f} (Limit: {alert['threshold_value']:.2f}) -> {alert['message']}\n"
        )
        with open(LOG_FILE_PATH, "a", encoding="utf-8") as f:
            f.write(log_line)
    except Exception as e:
        print(f"[DB Error] record_alert: {e}")

async def get_recent_history(limit: int = 60):
    try:
        async with aiosqlite.connect(DB_PATH) as db:
            db.row_factory = aiosqlite.Row
            cursor = await db.execute("""
                SELECT * FROM metrics_history ORDER BY id DESC LIMIT ?
            """, (limit,))
            rows = await cursor.fetchall()
            return [dict(r) for r in reversed(rows)]
    except Exception as e:
        print(f"[DB Error] get_recent_history: {e}")
        return []

async def get_recent_alerts(limit: int = 50):
    try:
        async with aiosqlite.connect(DB_PATH) as db:
            db.row_factory = aiosqlite.Row
            cursor = await db.execute("""
                SELECT * FROM alerts_history ORDER BY id DESC LIMIT ?
            """, (limit,))
            rows = await cursor.fetchall()
            return [dict(r) for r in rows]
    except Exception as e:
        print(f"[DB Error] get_recent_alerts: {e}")
        return []
