@echo off
REM ============================================================================
REM Kernel Monitor - 1-Click Windows Launcher (with WSL Linux Kernel Collector)
REM OSSP (25CS2104E) - Team 16
REM ============================================================================

echo ============================================================================
echo   LINUX KERNEL MONITOR - OSSP (25CS2104E) TEAM 16
echo   Saahas Vaddi ^| Lohith Sanapala ^| Rithwik Rapaka
echo ============================================================================

echo.
echo [1/2] Compiling C Kernel Collector in WSL Ubuntu...
wsl make -C /mnt/e/OSSP/collector clean all

echo.
echo [2/2] Starting Kernel Monitor Backend & Serving Dashboard...
echo Dashboard URL: http://localhost:8000
echo.

start http://localhost:8000
python -m uvicorn backend.app.main:app --host 0.0.0.0 --port 8000
pause
