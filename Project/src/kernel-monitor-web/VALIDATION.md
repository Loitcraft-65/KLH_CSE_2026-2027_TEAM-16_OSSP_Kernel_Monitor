# Validation performed

- Compiled collector.c on Linux with GCC using -std=c11 -O2 -Wall -Wextra -Wpedantic; no warnings.
- Ran the C collector against live /proc data and validated its JSON fields, memory arithmetic, CPU ranges and process-count consistency.
- Checked both replacement JavaScript files with node --check.
- Exercised every existing dashboard API endpoint with the C collector running: snapshot, system, history, alerts, thresholds, CSV and JSON exports.
- Verified separate cooldowns for simultaneous CPU and memory alerts and rejection of invalid thresholds.
- Verified that the bridge reports C child termination and that a missing collector binary produces a useful error and nonzero backend exit.
- Verified that all client source and dependency files are byte-for-byte unchanged from the upload.

The frontend was not installed, built or visually tested in this environment. API compatibility was tested; run npm ci and npm run dev on your Ubuntu/WSL setup. The source package excludes installed dependencies and compiled binaries. System-call tracing is blocked in the review environment; no trace result is claimed.
