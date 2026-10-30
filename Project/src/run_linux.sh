#!/usr/bin/env bash
# ============================================================================
# Kernel Monitor - 1-Click Linux / Ubuntu Launcher
# OSSP (25CS2104E) - Team 16
# ============================================================================

set -e

echo "=== Building C Kernel Collector ==="
make -C collector clean all

echo ""
echo "=== Starting Kernel Monitor Backend & Web Dashboard ==="
echo "Access Dashboard at: http://localhost:8000"
echo "Press Ctrl+C to terminate."
echo ""

python3 -m uvicorn backend.app.main:app --host 0.0.0.0 --port 8000 --reload
