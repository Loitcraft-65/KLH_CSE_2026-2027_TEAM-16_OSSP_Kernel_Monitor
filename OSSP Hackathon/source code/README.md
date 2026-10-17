# AeroSearch: Parallel Multi-Threaded File Search Engine

A high-concurrency file search engine built on a **C POSIX Threads** core, connected via **FastAPI WebSocket Streaming** to an **Interactive Cyber-Industrial Web UI**.

Developed for the **OSSP Hackathon** under the topic **"Parallel File Search using Threads"**.

### **Team - 16**
| Team Member | Registration / Roll Number |
| :--- | :--- |
| **Lohith Sanapala** | **2520030023** |
| **Saahas Vaddi** | **2520030256** |
| **Rapaka Rithwik** | **2520030427** |

📖 **Full Academic Project Documentation**: See [PROJECT_DOCUMENTATION.md](PROJECT_DOCUMENTATION.md) for in-depth operating system design, queue theory, mathematical Amdahl's Law derivation, and empirical benchmarks.

---

## 🌟 Key Highlights

- **Native C Thread Pool Engine**: Implemented in C using POSIX Threads (`pthread_create`, `pthread_mutex_t`, `pthread_cond_t`). Statically compiled for zero-dependency native Windows and Linux execution.
- **Dynamic Task Queue Architecture**: Producer-Consumer model where worker threads concurrently pop directories/files, balance the load dynamically, and expand subdirectories without locking the entire tree.
- **Real-Time Telemetry & WebSocket Streaming**: Streams worker thread status (`IDLE`, `SCANNING`, `MATCHING`, `DONE`), files scanned, and match context snippets at 16–30 FPS directly to the frontend.
- **Dual Matching Modes**: Deep full-text grep search inside file content (with binary file detection and line-number extraction) and filename wildcard/substring search.
- **Concurrency Benchmark Suite**: Benchmarks search speed across 1, 2, 4, 8, and 16 threads, plotting measured speedup curves against **Amdahl's Law** theoretical limits.
- **One-Click Mock Dataset Generator**: Instantly builds nested folder structures containing hundreds of realistic code, log, config, and documentation files with injected needle tokens for immediate evaluation.

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                       Frontend UI                           │
│  - Real-Time Thread HUD (Status, Files, Matches, Live File) │
│  - Filterable Results Explorer (Content & Filename matches) │
│  - Interactive Amdahl's Law Speedup Canvas Chart            │
└──────────────┬──────────────────────────────▲───────────────┘
               │ HTTP / REST                  │ WebSocket
               ▼                              │ (JSON Event Stream)
┌─────────────────────────────────────────────┴───────────────┐
│                    FastAPI Backend                          │
│  - WebSocket Server (/ws/search)                            │
│  - Benchmarking Orchestrator (/api/benchmark)               │
│  - Test Dataset Generator (/api/generate-data)              │
└──────────────┬──────────────────────────────────────────────┘
               │ Subprocess pipe (stdin/stdout)
               ▼
┌─────────────────────────────────────────────────────────────┐
│             C Core Search Engine (POSIX Threads)            │
│  ┌───────────────────────────────────────────────────────┐  │
│  │           Shared Thread-Safe TaskQueue                │  │
│  │   (pthread_mutex_t lock + pthread_cond_t not_empty)   │  │
│  └───────────────────────────┬───────────────────────────┘  │
│                              │ Pull Tasks                   │
│        ┌─────────────┬───────┴─────┬─────────────┐          │
│        ▼             ▼             ▼             ▼          │
│   Worker #0     Worker #1     Worker #2     Worker #N       │
│  [File I/O]    [File I/O]    [File I/O]    [File I/O]       │
│  [Pattern]     [Pattern]     [Pattern]     [Pattern]        │
│  [Telemetry]   [Telemetry]   [Telemetry]   [Telemetry]      │
└─────────────────────────────────────────────────────────────┘
```

---

## ⚡ Concurrency & Synchronization Details

### 1. Producer-Consumer Dynamic Task Queue
- The root directory is pushed to the queue.
- Any worker thread that encounters a subdirectory pushes its child entries back into the queue.
- Any worker that encounters a regular file executes pattern matching on it.
- **Termination Guarantee**: The queue detects completion when `queue_count == 0 && active_workers == 0`, broadcasting a shutdown condition to prevent deadlocks or premature exits.

### 2. Thread Safety
- `pthread_mutex_t queue_lock`: Guards task push and pop operations.
- `pthread_cond_t not_empty`: Coordinates worker sleep and wakeup cycles.
- `pthread_mutex_t worker_lock`: Isolates per-thread telemetry updates to eliminate data races.
- `pthread_mutex_t print_lock`: Guarantees atomic stdout JSON event emission.

---

## 🚀 Quickstart & How to Run

### Requirements
- **Python 3.10+**
- Installed libraries: `fastapi`, `uvicorn`, `websockets`, `psutil` (already configured in workspace)
- Native C binary `c_engine/parallel_search.exe` is already pre-compiled with `-O3 -static`.

### 1. Launch Everything in One Command
```powershell
python run.py
```
This automatically:
1. Verifies/compiles the C binary.
2. Pre-generates the test dataset (800 files) if not already present.
3. Starts the server at `http://127.0.0.1:8000/`.
4. Opens your default web browser to the application!

### 2. Manual CLI Execution (Standalone C Engine)
You can also run the C engine directly in the terminal:
```bash
./c_engine/parallel_search.exe -d ./test_dataset -p "HACKATHON_CHAMPION_2026" -t 8
```
Or with real-time JSON streaming:
```bash
./c_engine/parallel_search.exe -d ./test_dataset -p "HACKATHON_CHAMPION_2026" -t 8 --stream
```

---

## 📊 Benchmarking & Amdahl's Law

The built-in benchmark runner tests speed across 1, 2, 4, 8, and 16 threads:
$$\text{Speedup } S(n) = \frac{T_1}{T_n}$$
$$\text{Amdahl's Law } S(n) = \frac{1}{(1 - P) + \frac{P}{n}}$$
Where $P$ represents the parallelizable portion of the search pipeline (file reading & pattern matching).

The web interface plots measured performance alongside theoretical predictions to demonstrate multi-core CPU scaling.

---

## 📁 Repository Structure

```
OSSP-Hackathon/
├── c_engine/                    # Native C Core Engine
│   ├── main.c                   # CLI parser, thread management & telemetry reporter
│   ├── queue.h / queue.c        # Thread-safe task queue (POSIX Mutex & Cond)
│   ├── search.h / search.c      # Parallel directory traversal & content search
│   ├── Makefile                 # Linux / WSL build config
│   ├── build.bat                # Windows build script
│   └── parallel_search.exe      # Statically compiled native Windows binary
├── backend/                     # Python FastAPI Backend
│   ├── main.py                  # WebSocket & REST endpoints
│   ├── c_bridge.py              # Subprocess bridge & async stream parser
│   ├── benchmark.py             # Multi-thread speedup & latency benchmark runner
│   ├── data_gen.py              # Mock file dataset generator
│   └── requirements.txt         # Dependencies
├── frontend/                    # Modern Cyber-Industrial Web UI
│   ├── index.html               # Main dashboard with Thread HUD & Results Table
│   ├── styles.css               # Glassmorphism, animations & design system
│   ├── app.js                   # WebSocket manager & dynamic thread cards
│   └── benchmark_chart.js       # Retina Canvas Speedup vs Amdahl's chart
├── test_dataset/                # Generated sample dataset
├── run.py                       # One-click launcher
└── README.md                    # Project documentation
```
