@echo off
echo Compiling Parallel File Search Engine (C POSIX Threads)...
gcc -Wall -Wextra -O3 -static main.c queue.c search.c -o parallel_search.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] parallel_search.exe built successfully!
) else (
    echo [ERROR] Compilation failed.
)
