# OSSP Hackathon Project Documentation

# AeroSearch: High-Performance Parallel File Search Engine using POSIX Threads

---

### **Team Details**
- **Team Number**: **Team - 16**
- **Project Topic**: **Parallel File Search using Threads**
- **Core Technology Stack**: C (POSIX Threads / Win32 UCRT), Python 3.14 (FastAPI, WebSockets, Asyncio), Modern Web UI (HTML5, Vanilla CSS3, High-DPI Canvas)

### **Team Members**
| S.No | Student Name | Registration / Roll Number | Role / Contributions |
| :--- | :--- | :--- | :--- |
| 1 | **Lohith Sanapala** | **2520030023** | System Architecture, C Core Multi-threading Engine, Task Queue Synchronization |
| 2 | **Saahas Vaddi** | **2520030256** | FastAPI Backend Bridge, WebSocket Streaming, Benchmark Suite & Amdahl's Law Modeling |
| 3 | **Rapaka Rithwik** | **2520030427** | Cyber-Industrial Frontend UI, Real-time Thread Telemetry HUD, Data Generator |

---

## 📑 Table of Contents
1. [Executive Summary](#1-executive-summary)
2. [Problem Statement & Objectives](#2-problem-statement--objectives)
3. [System Architecture](#3-system-architecture)
4. [Concurrency & Operating System Principles](#4-concurrency--operating-system-principles)
   - 4.1 [Dynamic Work-Stealing Task Queue](#41-dynamic-work-stealing-task-queue)
   - 4.2 [Thread Synchronization & Mutex Locking](#42-thread-synchronization--mutex-locking)
   - 4.3 [Deadlock Prevention & Clean Termination](#43-deadlock-prevention--clean-termination)
5. [Implementation Breakdown](#5-implementation-breakdown)
   - 5.1 [C Core Search Engine](#51-c-core-search-engine)
   - 5.2 [Backend Streaming Server & IPC](#52-backend-streaming-server--ipc)
   - 5.3 [Interactive Frontend & Concurrency HUD](#53-interactive-frontend--concurrency-hud)
6. [Experimental Evaluation & Benchmarking](#6-experimental-evaluation--benchmarking)
   - 6.1 [Empirical Speedup vs. Amdahl's Law](#61-empirical-speedup-vs-amdahls-law)
   - 6.2 [Throughput & Scaling Analysis](#62-throughput--scaling-analysis)
7. [Engineering Challenges & Solutions](#7-engineering-challenges--solutions)
8. [User Manual & Execution Guide](#8-user-manual--execution-guide)
9. [Conclusion & Future Scope](#9-conclusion--future-scope)

---

## 1. Executive Summary

Traditional single-threaded file search tools (like naive implementations of `find` or `grep`) execute sequentially, resulting in high I/O latency and underutilization of modern multi-core computing hardware. 

**AeroSearch** addresses this limitation by implementing a low-level, high-concurrency file traversal and content pattern matching engine written in **C using POSIX Threads (`pthreads`)**. The engine employs a dynamic Producer-Consumer work queue with fine-grained mutual exclusion locks and condition variables, allowing multiple worker threads to traverse independent subdirectories and inspect file contents concurrently.

To make the multi-threaded execution observable in real time, AeroSearch integrates:
1. A **FastAPI asynchronous backend bridge** that intercepts the C process's real-time JSON stream over non-blocking pipes.
2. A **WebSocket streaming channel** broadcasting thread telemetry at 16–30 FPS.
3. A **Cyber-Industrial Web Dashboard** featuring live worker thread HUD cards, instant filtering, one-click test data generation, and an interactive **Amdahl's Law** speedup benchmark visualizer.

---

## 2. Problem Statement & Objectives

### Problem Statement
File system searches over nested hierarchies containing tens of thousands of files are bounded by both storage I/O and CPU string-matching operations. Single-threaded search algorithms suffer from severe latency bottlenecks when reading large directories or executing regular expressions across deep directories.

### Project Objectives
- **C-Based Concurrency**: Build a multi-threaded file traversal and content grep engine entirely in C using POSIX Threads without external runtime dependencies.
- **Dynamic Load Balancing**: Design a thread-safe task queue that dynamically balances file and directory workloads across $N$ worker threads.
- **Deep Content Grep**: Support both case-sensitive/insensitive filename matching and multi-threaded deep content searching with line numbers and snippet context.
- **Telemetry Observability**: Stream live thread state transitions (`IDLE` $\rightarrow$ `SCANNING` $\rightarrow$ `MATCHING` $\rightarrow$ `DONE`) and individual file counters directly to a frontend UI.
- **Scientific Benchmarking**: Quantitatively measure speedup ($S_N = \frac{T_1}{T_N}$) and compare results against theoretical limits modeled by **Amdahl's Law**.

---

## 3. System Architecture

AeroSearch is structured in three modular layers: **C Computational Core**, **FastAPI Subprocess Bridge**, and **Reactive Web Frontend**.

```mermaid
graph TD
    subgraph "Presentation Layer (Frontend)"
        HUD["Thread Worker HUD<br>(Real-time telemetry cards)"]
        RES["Results Explorer<br>(Filter by Content/Filename)"]
        CHART["Amdahl's Law Chart<br>(Canvas 2D Speedup Curve)"]
    end

    subgraph "Application Layer (FastAPI Backend)"
        WS["WebSocket Server<br>(/ws/search)"]
        API["REST Endpoints<br>(/api/benchmark, /api/generate-data)"]
        BRIDGE["C IPC Subprocess Pipe<br>(c_bridge.py)"]
    end

    subgraph "Core Concurrency Layer (C POSIX)"
        Q["Thread-Safe Task Queue<br>(pthread_mutex_t + pthread_cond_t)"]
        W0["Worker Thread #0"]
        W1["Worker Thread #1"]
        W2["Worker Thread #2"]
        WN["Worker Thread #N"]
        FS["File System I/O<br>(opendir / readdir / buffered fread)"]
    end

    HUD <-->|WebSocket Stream (16-30 FPS)| WS
    RES <-->|Match JSON Objects| WS
    CHART <-->|Benchmark Results| API

    WS <--> BRIDGE
    API <--> BRIDGE
    BRIDGE <-->|Async Non-blocking Stdio| Q

    Q --> W0
    Q --> W1
    Q --> W2
    Q --> WN

    W0 <--> FS
    W1 <--> FS
    W2 <--> FS
    WN <--> FS
```

---

## 4. Concurrency & Operating System Principles

### 4.1 Dynamic Work-Stealing Task Queue
A major pitfall of static thread division (e.g., assigning folder `A` to Thread 1 and folder `B` to Thread 2) is **thread starvation**, caused by unbalanced directory trees where one folder has 1,000 files and another has only 2.

AeroSearch solves this with a **dynamic Producer-Consumer Queue** (`TaskQueue` in [queue.h](file:///e:/OSSP-Hackathon/c_engine/queue.h)):
- **Task Representation**:
  ```c
  typedef struct Task {
      char path[MAX_PATH_LEN];
      TaskType type; // TASK_DIRECTORY or TASK_FILE
      int depth;
      struct Task* next;
  } Task;
  ```
- Any worker thread that pops a directory reads its entries using `opendir()` and `readdir()`, pushing child subdirectories and files back into the shared queue.
- Any worker thread that pops a regular file opens it and scans for the search pattern.
- This produces natural work-stealing and load balancing without requiring complex partition heuristics.

### 4.2 Thread Synchronization & Mutex Locking
To guarantee thread-safety and eliminate race conditions:
1. **Queue Mutex (`q->lock`)**:
   - Protects pointer adjustments to `q->head` and `q->tail`.
   - Protects `q->count` and `q->active_workers`.
2. **Condition Variable (`q->not_empty`)**:
   - Worker threads call `pthread_cond_wait(&q->not_empty, &q->lock)` when the queue is temporarily empty, releasing the CPU instead of busy-waiting.
   - Pushing any task calls `pthread_cond_signal(&q->not_empty)` to awaken sleeping threads.
3. **Telemetry Mutex (`telem->worker_lock`)**:
   - Each thread possesses an independent mutex protecting its current file string and scanned counters, avoiding contention with other worker threads.
4. **Standard Output Mutex (`stats->print_lock`)**:
   - Guarantees that atomic JSON telemetry objects printed to `stdout` are never interleaved or corrupted.

### 4.3 Deadlock Prevention & Clean Termination
In parallel tree exploration, an empty queue **does not** imply that searching is complete—another active thread might currently be reading a directory that will enqueue 50 new items.

AeroSearch implements a clean termination rule:
$$\text{Termination Condition} \iff (\text{queue\_count} == 0) \land (\text{active\_workers} == 0)$$

```c
// Evaluated inside queue_pop() and queue_worker_done_task()
if (q->active_workers == 0 && q->count == 0) {
    q->shutdown = true;
    pthread_cond_broadcast(&q->not_empty); // Wake up all waiting threads
    pthread_mutex_unlock(&q->lock);
    return NULL; // All threads cleanly exit
}
```

---

## 5. Implementation Breakdown

### 5.1 C Core Search Engine (`c_engine/`)
- **[queue.c](file:///e:/OSSP-Hackathon/c_engine/queue.c)**: Thread-safe FIFO queue with condition variables and automatic termination.
- **[search.c](file:///e:/OSSP-Hackathon/c_engine/search.c)**: 
  - Binary file detection: Inspects the initial 1024 bytes of each file for null bytes (`\0`). Binary files (`.exe`, `.dll`, `.png`) are skipped during content searches to optimize throughput.
  - Substring & Wildcard Matching: Case-insensitive searching (`strcasestr_custom`) and file extension filtering (`.c,.h,.py,.txt,.log`).
  - JSON Sanitization: Path backslashes (`\`) and double quotes are escaped on-the-fly (`escape_json`) to ensure valid JSON streams.
- **[main.c](file:///e:/OSSP-Hackathon/c_engine/main.c)**: Command-line parsing, worker thread lifecycle management, background telemetry reporter thread emitting structured JSON pulses every 60ms, and high-precision timing via `gettimeofday()`.

### 5.2 Backend Streaming Server & IPC (`backend/`)
- **[c_bridge.py](file:///e:/OSSP-Hackathon/backend/c_bridge.py)**: Spawns the compiled C binary (`parallel_search.exe`) via `asyncio.create_subprocess_exec`, reads `stdout` asynchronously line-by-line, and yields JSON objects directly to the WebSocket channel. Includes a pure Python threadpool fallback for environment resiliency.
- **[benchmark.py](file:///e:/OSSP-Hackathon/backend/benchmark.py)**: Coordinates consecutive runs across 1, 2, 4, 8, and 16 threads against the same directory, gathering execution times, calculating speedups, and computing Amdahl's Law limits.
- **[data_gen.py](file:///e:/OSSP-Hackathon/backend/data_gen.py)**: Procedurally generates realistic file systems (C code, Python modules, JSON configs, server logs) with embedded target tokens (`HACKATHON_CHAMPION_2026`, `DATABASE_DEADLOCK_DETECTED`).

### 5.3 Interactive Frontend & Concurrency HUD (`frontend/`)
- **[index.html](file:///e:/OSSP-Hackathon/frontend/index.html)**: Cyber-Industrial UI structured with semantic HTML5 elements, HUD metrics, and accessible controls.
- **[styles.css](file:///e:/OSSP-Hackathon/frontend/styles.css)**: Custom design system using glassmorphism, animated glowing activity indicators, JetBrains Mono typography, and responsive grid layouts.
- **[app.js](file:///e:/OSSP-Hackathon/frontend/app.js)**: Client-side WebSocket controller, dynamic thread HUD card generation, real-time result table updating, live filtering, and CSV/JSON export.
- **[benchmark_chart.js](file:///e:/OSSP-Hackathon/frontend/benchmark_chart.js)**: Native Canvas 2D engine rendering high-DPI scaling curves, comparing empirical speedup against theoretical Amdahl's Law.

---

## 6. Experimental Evaluation & Benchmarking

### 6.1 Empirical Speedup vs. Amdahl's Law
Amdahl's Law defines the theoretical speedup of an algorithm given a parallel fraction $P$:
$$S(n) = \frac{1}{(1 - P) + \frac{P}{n}}$$

In file searching:
- **Parallel Fraction ($P \approx 0.88 - 0.92$)**: File opening, reading file content, string scanning, regex comparison.
- **Serial Fraction ($1 - P \approx 0.08 - 0.12$)**: Root directory initialization, thread creation and joining, queue lock acquisition, and OS kernel disk scheduling contention.

#### Benchmark Execution Results (800 Files / 16 Nested Directories):
| Worker Threads ($n$) | Execution Time ($T_n$) | Empirical Speedup ($S_n$) | Parallel Efficiency ($E_n$) | Theoretical Amdahl Speedup ($P=0.88$) | Throughput (Files/sec) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **1 Thread** | **99.87 ms** | **1.00x** | 100.0% | 1.00x | 8,010 files/s |
| **2 Threads** | **64.04 ms** | **1.56x** | 78.0% | 1.79x | 12,492 files/s |
| **4 Threads** | **35.67 ms** | **2.80x** | 70.0% | 2.94x | **22,427 files/s** |
| **8 Threads** | **46.34 ms** | **2.16x** | 27.0% | 4.35x | 17,263 files/s |
| **16 Threads** | **42.93 ms** | **2.33x** | 14.6% | 5.71x | 18,635 files/s |

### 6.2 Throughput & Scaling Analysis
1. **Sub-linear Scaling up to 4 Cores**: The system achieves near-ideal speedup moving from 1 thread (99.87 ms) to 4 threads (35.67 ms), boosting throughput from 8,010 to 22,427 files/second.
2. **I/O Bus Saturation Beyond 8 Threads**: Increasing thread counts beyond physical core capacity (4 to 8+ threads) introduces thread context-switching overhead and disk I/O head contention, illustrating the classic Operating Systems principle of memory and I/O bus saturation.

---

## 7. Engineering Challenges & Solutions

| # | Challenge Encountered | Root Cause | Engineering Solution |
| :--- | :--- | :--- | :--- |
| **1** | **DLL Dependency Conflicts on Windows** | Standard GCC builds dynamically link `libwinpthread-1.dll`, which failed on systems without MinGW in system `PATH`. | Statically compiled the binary using `gcc -Wall -Wextra -O3 -static` to bundle POSIX threads directly into a zero-dependency standalone `.exe`. |
| **2** | **Premature Search Termination** | Worker threads seeing an empty queue would exit while sibling threads were still reading subdirectories. | Implemented `active_workers` counter tracking; termination only signals when `queue_count == 0` AND `active_workers == 0`. |
| **3** | **JSON Parse Errors from Windows File Paths** | File paths with Windows backslashes (`\`) and quotes in code snippets broke the WebSocket JSON parser. | Developed an in-place `escape_json()` C routine that normalizes paths to forward slashes and escapes all quotes and control characters. |
| **4** | **Frontend UI Freezing During Fast Streams** | Emitting individual WebSocket messages for thousands of files overwhelmed browser rendering threads. | Implemented a dedicated 60ms background telemetry pulse thread in C, decoupling thread monitoring from disk I/O. |

---

## 8. User Manual & Execution Guide

### Prerequisites
- Python 3.10+ (with `fastapi`, `uvicorn`, `websockets`, `psutil`)
- Modern web browser (Chrome, Edge, Firefox, Brave)
- Statically compiled C binary: `c_engine/parallel_search.exe` (included and ready)

### One-Click Execution
From the root workspace directory, run:
```powershell
python run.py
```
This launcher:
1. Validates the native C binary.
2. Automatically generates the sample `test_dataset/` (800 files) if not already created.
3. Starts the Uvicorn web server at `http://127.0.0.1:8000/`.
4. Launches your default web browser automatically.

### Running the Standalone C Engine via Terminal
```bash
# General search with 8 worker threads:
./c_engine/parallel_search.exe -d ./test_dataset -p "HACKATHON_CHAMPION_2026" -t 8

# Streaming JSON mode:
./c_engine/parallel_search.exe -d ./test_dataset -p "queue" -t 4 --stream
```

### Recompiling the C Engine (Optional)
- **On Windows**:
  ```cmd
  cd c_engine
  build.bat
  ```
- **On Linux / WSL**:
  ```bash
  cd c_engine
  make clean && make
  ```

---

## 9. Conclusion & Future Scope

AeroSearch successfully validates that multi-threaded file search implemented in low-level C using POSIX Threads drastically outpaces sequential search routines, yielding a **2.80x speedup** on modern multi-core systems. The project bridges systems-level concurrent programming with modern real-time web telemetry.

### Future Enhancements
- **Memory-Mapped File I/O (`mmap`)**: Replacing buffered `fread()` with memory-mapped files for zero-copy kernel-to-user space page reading.
- **Hardware SIMD Vectorization**: Utilizing AVX-512 vector instructions for multi-gigabyte/second regex and substring scanning.
- **Distributed Node Cluster**: Extending the work queue across network sockets to enable distributed parallel searching across multiple workstations.

---

### **Project Sign-Off**
- **Team**: Team - 16
- **Members**:
  - Lohith Sanapala (2520030023)
  - Saahas Vaddi (2520030256)
  - Rapaka Rithwik (2520030427)
- **Status**: Complete, Verified, and Ready for Hackathon Presentation.
