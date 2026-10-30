# Linux Kernel Monitor
**Operating Systems and Systems Programming (25CS2104E)**  
**Academic Year 2026–27, Term-I | Section: 4 | Team No.: 16**

---

### Team Members & Responsibilities
| Roll Number | Student Name | Individual Responsibility |
|:---:|:---:|:---|
| **2520030023** | **Lohith Sanapala** | **Kernel-level data collection module in C** (`/proc` parsers, system calls `sysinfo()` & `getrusage()`, kernel thread discovery) |
| **2520030256** | **Saahas Vaddi** | **Backend service, API, and database integration** (FastAPI, WebSocket streaming, SQLite persistence, socket IPC) |
| **2520030427** | **Rithwik Rapaka** | **Visualisation and alerting logic** (Real-time telemetry dashboard, Chart.js telemetry, threshold rules engine) |

---

## 1. Why This Is a "Kernel Monitor" (Not a Task Manager)

Standard tools like Windows Task Manager or Linux `top` focus on **user-space processes** (which app is running, user memory footprints).  
In contrast, this project is a **Kernel Subsystem Telemetry Monitor** designed for low-level systems programming analysis. It inspects:

1. **CPU Scheduler Subsystem (`/proc/stat` & `/proc/loadavg`)**:
   - Distinctly tracks **Kernel Space CPU time (`system`)**, User Space CPU time (`user`), Hardware Interrupt time (`irq`), and Kernel SoftIRQ time (`softirq`).
   - Measures **Kernel Context Switches (`ctxt`)** per second (frequency at which the scheduler switches thread execution contexts).
   - Tracks runnable vs total kernel scheduling entities.

2. **Memory & Virtual Memory Subsystem (`/proc/meminfo` & `/proc/vmstat`)**:
   - **Kernel SLAB Caches (`Slab`)**: Divides memory into `SReclaimable` and `SUnreclaim` to observe internal kernel object caching.
   - **Page Tables & Kernel Stacks**: Quantifies physical RAM consumed strictly by MMU page table structures and thread kernel stacks.
   - **Virtual Memory Paging**: Tracks **Minor Page Faults** (RAM address space mapped) versus **Major Page Faults** (disk page-ins required).
   - **Dirty Memory Buffers**: Memory marked dirty awaiting kernel pdflush / flusher thread synchronization.

3. **Virtual File System & Block Layer (`/proc/sys/fs/file-nr` & `/proc/diskstats`)**:
   - Monitors active allocated kernel file descriptors against the system-wide limit (`fs.file-max`).
   - Tracks queue depth (I/O in flight) and sector throughput.

4. **Linux System Calls (`sysinfo()` & `getrusage()`)**:
   - Gathers uptime, load averages, and memory scale units directly via `sysinfo()`.
   - Uses `getrusage(RUSAGE_SELF)` to inspect voluntary vs involuntary context switches and CPU time of the monitoring agent.

5. **Kernel Threads Inspection (`kthreads`)**:
   - Distinguishes genuine **Kernel Threads** (`kthreadd` PID 2, `kworker/*`, `ksoftirqd/*`, `rcu_preempt`, `migration/*`) from user-space processes by evaluating PPID and user virtual memory mapping (`VmSize`).

---

## 2. System Architecture

```
e:\OSSP\
├── collector/                  # [Lohith Sanapala] Low-Level C Kernel Collector
│   ├── include/
│   │   ├── collector.h         # Metric schemas & telemetry structures
│   │   ├── proc_parser.h       # /proc/stat, /proc/meminfo, /proc/vmstat, file-nr
│   │   ├── syscalls.h          # sysinfo() and getrusage() system calls
│   │   └── kthreads.h          # Kernel thread classification & context switches
│   ├── src/
│   │   ├── proc_parser.c
│   │   ├── syscalls.c
│   │   ├── kthreads.c
│   │   └── main.c              # Terminal ANSI view & JSON streaming engine
│   └── Makefile                # Clean C build system
│
├── backend/                    # [Saahas Vaddi] Backend Service & Database
│   ├── app/
│   │   ├── config.py           # Threshold configuration models
│   │   ├── database.py         # SQLite persistence & historical logging
│   │   ├── alerting.py         # Rules engine comparing kernel thresholds
│   │   ├── collector_bridge.py # Process bridge connecting C collector
│   │   └── main.py             # FastAPI REST endpoints & WebSocket server
│   └── requirements.txt
│
├── frontend/                   # [Rithwik Rapaka] Visualization & Alerting UI
│   ├── index.html              # Cyber-styled single page dashboard
│   ├── css/
│   │   └── style.css           # Glassmorphic dark theme, CSS variables, progress bars
│   └── js/
│       ├── api.js              # REST & WebSocket client with auto-reconnect
│       ├── charts.js           # Real-time Chart.js telemetry graphs
│       └── app.js              # Live DOM updates, filtering & threshold tuning
│
├── logs/                       # Persistent audit trail
│   └── kernel_monitor.log
│
├── run_linux.sh                # 1-click Linux / WSL launcher
├── run_windows.bat             # 1-click Windows launcher
└── Makefile                    # Root makefile
```

---

## 3. How to Run the Project

### Option A: On Windows (with WSL Ubuntu)
Simply double click `run_windows.bat` or run in PowerShell:
```powershell
.\run_windows.bat
```
This automatically compiles the C collector in WSL Ubuntu, starts the backend, and opens your browser at **`http://localhost:8000`**.

### Option B: On Native Ubuntu / Linux
```bash
# 1. Build and launch backend + web dashboard
./run_linux.sh

# 2. Or run the C Collector directly in Terminal Mode
make -C collector all
./kernel_collector --terminal
```

---

## 4. REST API & WebSocket Endpoints

| Method | Endpoint | Description |
|:---:|:---|:---|
| `GET` | `/` | Web Telemetry Dashboard |
| `WS` | `/ws/live` | Real-time 1Hz WebSocket telemetry stream |
| `GET` | `/api/status` | Kernel version and service status |
| `GET` | `/api/metrics/live` | Current live snapshot of all kernel subsystems |
| `GET` | `/api/metrics/history` | Historical time-series metrics from SQLite |
| `GET` | `/api/alerts` | Active alerts and alert log history |
| `GET` | `/api/thresholds` | Current alert trigger limits |
| `POST`| `/api/thresholds` | Update dynamic threshold limits |
| `GET` | `/api/logs` | View recent entries from `logs/kernel_monitor.log` |
