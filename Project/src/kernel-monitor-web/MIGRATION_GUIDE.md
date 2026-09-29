# Exactly what to remove, add, and replace

## 1. Choose how to apply this package

Simplest: extract this entire package into a new folder, open kernel-monitor-web, and follow README.md.

For manual integration into your current project, apply the four source changes below. All paths are relative to kernel-monitor-web. Back up your old files before replacing them.

| Action | Exact path | What to do |
| --- | --- | --- |
| Add | collector/collector.c | Create collector beside client and server. Copy the supplied C source into it. |
| Add | collector/Makefile | Copy the supplied Makefile into collector. |
| Replace entire file | server/collectors.js | Remove the old JavaScript /proc collector and simulator. Use the supplied bridge file. |
| Replace entire file | server/index.js | Use the supplied backend that receives samples from C. |
| Keep | client/ | No frontend edits are required for C integration. |
| Keep | server/package.json and package-lock.json | No additional npm dependency is required. |
| Update documentation | README.md | Use the supplied run instructions; MIGRATION_GUIDE.md is this guide. |

The compiled collector/kernel_collector file is created by make. Do not create it in an editor or rename collector.c to kernel_collector.

## 2. What is removed from server/collectors.js

The old implementations of readCpuTimesFromProc, readCpuTimesFromOs, getCpuUsagePercent, getMemoryStats, readProcessStatus, listProcesses, simulateProcesses, and getSystemInfo are removed from this file, along with previous-sample maps and simulation data.

The replacement contains startCollector, getSnapshot, getCollectorError, and stopCollector. It runs C and receives its output. It does not parse /proc or calculate CPU percentages. Static machine labels in /api/system still use Node's os module; CPU, memory, process metrics and uptime come from C.

## 3. What changes in server/index.js

Remove the old imports getCpuUsagePercent, getMemoryStats, listProcesses, getSystemInfo and HAS_PROC. The replacement imports the bridge functions instead.

Remove SAMPLE_INTERVAL_MS and `setInterval(sample, SAMPLE_INTERVAL_MS)`. C now controls sampling. Leaving that interval in would create duplicate or mismatched samples.

The new sample(snapshot) callback records CPU/memory history and checks alerts whenever a complete C JSON line arrives. /api/snapshot returns the latest C object directly; it does not rescan processes for each browser.

The existing endpoint names are preserved:

- /api/system
- /api/snapshot
- /api/history
- /api/alerts
- /api/thresholds (GET and POST)
- /api/export.csv
- /api/export.json

Before the first sample, or after five seconds without fresh data, live endpoints return HTTP 503. A collector startup failure or exit stops the backend with an error instead of presenting stale values as live.

The new index also fixes alert cooldown tracking per alert type and rejects out-of-range threshold values. Changing browser refresh frequency does not change C's sampling interval.

## 4. What the C functions do

| C function | Responsibility |
| --- | --- |
| read_text | Calls open/read/close to read bounded /proc text; handles partial reads. |
| read_cpu | Reads the aggregate CPU counters from /proc/stat. |
| read_memory | Parses /proc/meminfo; returns bytes and available-memory-based usage. |
| read_process | Reads /proc/PID/status and stat; extracts name, state, RSS, threads and CPU ticks. |
| scan_processes | Enumerates numeric /proc directories using opendir/readdir/closedir. |
| emit | Serializes a sample as one JSON line and flushes stdout. |
| main | Takes a baseline, waits, calculates deltas, calls sysinfo/getrusage, and emits samples. |

CPU percentage = 100 * (1 - delta_idle / delta_total).
Process CPU percentage = 100 * delta_ticks / ticks_per_second / elapsed_seconds.

sysconf(_SC_CLK_TCK) queries the tick conversion instead of hard-coding 100. CLOCK_MONOTONIC measures elapsed intervals. New or reused PIDs get a zero baseline reading rather than a lifetime-based spike. Process CPU can exceed 100% when more than one core is used; aggregate CPU stays between 0% and 100%.

Process records are best-effort: processes can exit while being read and permissions can restrict visibility. totalProcesses and totalThreads describe successfully collected processes visible in this Linux environment.

## 5. System-call explanation for your course

| Call/API | Used for | Classification |
| --- | --- | --- |
| open, read, close | Read kernel-provided /proc files | C interfaces to Linux system calls; libc may implement open with openat. |
| sysinfo | Uptime, load averages, and memory/swap defaults | Linux system call. |
| getrusage(RUSAGE_SELF) | Collector CPU time, peak RSS and context-switch counts | System call; measures the collector itself. |
| opendir, readdir, closedir | Enumerate process directories | C library APIs backed by filesystem operations such as getdents64. |
| sysconf(_SC_CLK_TCK) | Obtain clock ticks per second | C library API, not itself a Linux system call. |
| clock_gettime | Measure elapsed time and timestamp samples | POSIX API, often served through vDSO on Linux. |
| nanosleep | Wait between samples | POSIX API backed by a sleep system call; actual syscall name can differ. |
| printf and fflush | Write each JSON record to stdout | C stdio APIs; underlying writes go to the IPC pipe. |

A pipe is IPC. Node creates the process and connects its stdout using spawn with stdio set to pipe. The C program does not explicitly call pipe(), fork(), or socket(). HTTP sockets remain in the Node-to-browser connection.

getrusage(RUSAGE_SELF) does NOT collect every process's resource usage. Per-process metrics come from /proc/PID files. sysinfo load averages are NOT CPU percentages.

## 6. Build and run on Ubuntu/WSL2

Install GCC and make if missing:

```bash
sudo apt update
sudo apt install build-essential
```

Confirm Node >=22.12 and npm are installed inside Ubuntu:

```bash
node --version
npm --version
```

From kernel-monitor-web:

```bash
make -C collector
./collector/kernel_collector --once
```

Inspect JSON in the terminal, then start the backend:

```bash
cd server
npm ci
npm start
```

From kernel-monitor-web in a second Ubuntu terminal:

```bash
cd client
npm ci
npm run dev
```

Use the Local URL printed by Vite. The initial dashboard may show disconnected for about one second while C takes its first measurement.

If applying these files to an existing Windows checkout, run npm ci inside Ubuntu in both folders. It replaces node_modules with platform-appropriate dependencies. Do not reuse the Windows binaries from the original ZIP.

You can verify the backend at http://localhost:5000/api/snapshot. It should include `"collector":"c"`, `"usingRealProc":true`, memory, processes, and collectorUsage.

For standalone continuous C output:

```bash
./collector/kernel_collector
```

Ctrl+C stops it. This standalone mode prints JSON, not a formatted terminal dashboard.

Optional syscall demonstration on a Linux installation where tracing is permitted:

```bash
strace -e trace=openat,read,close,getdents64,sysinfo,getrusage ./collector/kernel_collector --once
```

Tracing was blocked in the review environment. The project was compiled and tested without claiming a captured syscall trace.

## 7. Align the abstract with what is actually implemented

Your abstract currently describes two different designs: an entirely C terminal application and a C-plus-web application. This migration follows the second design.

Describe the project as a Linux system monitor with a C collector, Node.js backend and React dashboard. Replace "developed entirely in C" with "uses a C data-collection module" and remove claims that there is no web interface.

In the methodology, replace the collector-to-backend "local socket or REST API" wording with "a local IPC pipe carrying newline-delimited JSON". The browser still communicates with Node over HTTP REST endpoints. If your instructor specifically requires a socket between C and Node, that is an additional transport change; this implementation does not claim to provide it.

History is kept in memory and can be exported. Do not claim database integration, persistent automatic logging, disk-I/O monitoring, Docker deployment, or a formatted terminal dashboard until those are added. The uploaded PDF itself has not been edited.

## 8. Existing UI limitations

The existing process table displays the top 30 processes. The unchanged Overview page counts those displayed rows and their threads. The C API additionally provides totalProcesses and totalThreads; optional follow-up UI changes can use those for totals. Pinning cannot retain a process that falls outside the backend's top 30.

The UI does not yet render collectorUsage or loadAverage, but they are visible in /api/snapshot. The export endpoints preserve the original format: CPU/memory history in CSV; history and alerts in JSON.

No simulated Windows process list remains. WSL monitors its Linux environment. Sampling is approximately one second plus collection time, not a hard real-time guarantee.

## References

- Linux sysinfo: https://man7.org/linux/man-pages/man2/sysinfo.2.html
- Linux getrusage: https://man7.org/linux/man-pages/man2/getrusage.2.html
- Linux /proc: https://www.kernel.org/doc/html/latest/filesystems/proc.html
- Node child processes: https://nodejs.org/api/child_process.html
