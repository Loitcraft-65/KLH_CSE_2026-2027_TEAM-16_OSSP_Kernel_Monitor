# KLH CSE (2026-2027) Team 16: OSSP Hackathon

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/Loitcraft-65/KLH_CSE_2026-2027_TEAM-16_OSSP_Kernel_Monitor)
[![Run AeroSearch Parallel Benchmarks](https://github.com/Loitcraft-65/KLH_CSE_2026-2027_TEAM-16_OSSP_Kernel_Monitor/actions/workflows/run_benchmark.yml/badge.svg)](https://github.com/Loitcraft-65/KLH_CSE_2026-2027_TEAM-16_OSSP_Kernel_Monitor/actions/workflows/run_benchmark.yml)

## 🏆 Project: AeroSearch — Parallel File Search Engine using POSIX Threads

> **Hackathon Topic**: Parallel File Search using Threads  
> **Core Basis**: Native C (POSIX Threads / GCC) + FastAPI + Cyber-Industrial Web UI  
> **Team**: Team - 16

### **Team Members**
| S.No | Student Name | Registration / Roll Number | Assigned Role |
| :---: | :--- | :--- | :--- |
| 1 | **Lohith Sanapala** | **2520030023** | System Architecture, C Core Multi-Threading & Queue Synchronization |
| 2 | **Saahas Vaddi** | **2520030256** | FastAPI Backend Bridge, WebSocket Streaming & Amdahl Benchmarking |
| 3 | **Rapaka Rithwik** | **2520030427** | Cyber-Industrial Frontend HUD & Test Data Generator |

---

## ⚡ How to Run Directly in GitHub

### Option 1: One-Click Cloud Execution (GitHub Codespaces)
You can run and interact with the full web app directly in your browser without installing anything locally:
1. Click the **[Open in GitHub Codespaces](https://codespaces.new/Loitcraft-65/KLH_CSE_2026-2027_TEAM-16_OSSP_Kernel_Monitor)** button above.
2. Codespaces automatically:
   - Provisions a multi-core Linux VM.
   - Compiles the C multi-threaded engine (`c_engine/parallel_search`).
   - Starts the Python server and opens port `8000`.
   - Automatically opens the **AeroSearch Web UI** in a live browser tab!

### Option 2: Run Benchmarks in GitHub Actions
1. Go to the **[Actions Tab](https://github.com/Loitcraft-65/KLH_CSE_2026-2027_TEAM-16_OSSP_Kernel_Monitor/actions)**.
2. Select **"Run AeroSearch Parallel Benchmarks"**.
3. Click **"Run workflow"**.
4. GitHub cloud runners will execute the multi-threaded search across 1, 2, 4, 8, and 16 threads and print the Speedup vs Amdahl's Law table directly in the Job Summary!

---

## 📂 Repository Navigation

- **[OSSP Hackathon/source code/](<OSSP Hackathon/source code>)**: Complete source code (C Core, FastAPI Backend, Web UI).
- **[OSSP Hackathon/ documentation/](<OSSP Hackathon/ documentation>)**: Project documentation in **Word (.docx)**, **Printable HTML**, and **Markdown**.
- **[OSSP Hackathon/ PPT/](<OSSP Hackathon/ PPT>)**: Presentation slides (`AeroSearch_Team16_Presentation.pptx`).
