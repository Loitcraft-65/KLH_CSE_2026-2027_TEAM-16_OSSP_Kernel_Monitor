# Kernel Monitor: C collector migration

This is a complete source copy of your uploaded web project with its collector moved to C. The React client and npm dependency files are unchanged. The original upload is not needed to use this copy. Read MIGRATION_GUIDE.md for the exact changes.

## Requirements

- Ubuntu/Linux, including Ubuntu in WSL2. Run both Node servers inside Linux.
- GCC and make: `sudo apt update`, then `sudo apt install build-essential`.
- Node.js 22.12 or newer and npm installed inside that Linux environment. The uploaded Vite dependency requires Node ^20.19.0 or >=22.12.0; use 22.12+ for this project.
- Network access for the first npm dependency installation.

`node_modules` and compiled binaries are deliberately omitted. npm installs platform-appropriate dependencies, and make builds the collector on your machine.

## Run

From the extracted `kernel-monitor-web` folder:

```bash
make -C collector
./collector/kernel_collector --once
```

The second command waits about one second and prints one JSON sample.

Terminal 1, from the project root:

```bash
cd server
npm ci
npm start
```

Terminal 2, from the project root:

```bash
cd client
npm ci
npm run dev
```

Open the Local URL printed by Vite, normally http://localhost:5173.
The backend starts and stops the collector automatically. Do not start a separate collector for normal dashboard operation. Ctrl+C stops each server.

## Architecture

1. C reads /proc and calls sysinfo()/getrusage().
2. C calculates a fresh sample and prints one JSON line, approximately every second.
3. Node child_process.spawn connects C stdout to a pipe.
4. Node readline assembles complete lines, parses JSON, and updates history/alerts once per received sample.
5. React polls the existing HTTP endpoints. The API field names used by React are preserved.

The C process runs in user space. It is not a kernel module.

## Scope

CPU, memory and process metrics now come from C. sysinfo provides uptime, load averages, and memory defaults; /proc/meminfo supplies detailed memory data. getrusage(RUSAGE_SELF) exposes the collector's own resource usage under collectorUsage.

History and alerts remain bounded in-memory arrays (900 samples, 500 alerts). Exports are downloads, not automatic database or disk persistence. This version does not add disk I/O monitoring, Docker, or a formatted terminal interface. It implements the C-collector part of the abstract's web architecture.

The backend listens on local loopback for the course demo. An unchanged React client can use it from the same machine. WSL reports the Linux environment, not all native Windows processes.
