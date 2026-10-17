#!/usr/bin/env python3
"""
AeroSearch - Parallel File Search Engine Launcher
One-click unified runner for Backend and Frontend.
"""

import os
import sys
import subprocess
import webbrowser
import time
import uvicorn

WORKSPACE_DIR = os.path.dirname(os.path.abspath(__file__))
C_ENGINE_DIR = os.path.join(WORKSPACE_DIR, "c_engine")
C_EXE_PATH = os.path.join(C_ENGINE_DIR, "parallel_search.exe")
TEST_DATASET_DIR = os.path.join(WORKSPACE_DIR, "test_dataset")

def check_or_compile_c():
    if not os.path.exists(C_EXE_PATH):
        print("[*] C binary not found. Compiling native parallel_search.exe with gcc...")
        try:
            cmd = ["gcc", "-Wall", "-Wextra", "-O3", "-static", "main.c", "queue.c", "search.c", "-o", "parallel_search.exe"]
            res = subprocess.run(cmd, cwd=C_ENGINE_DIR, capture_output=True, text=True)
            if res.returncode == 0:
                print("[+] Successfully compiled c_engine/parallel_search.exe!")
            else:
                print(f"[!] Warning: Native gcc compilation returned {res.returncode}. Fallback will be used if needed.")
        except Exception as e:
            print(f"[!] Compiler note: {e}")
    else:
        print("[+] Native C Search Engine binary detected: parallel_search.exe")

def check_test_dataset():
    if not os.path.exists(TEST_DATASET_DIR):
        print("[*] Pre-generating sample test dataset with 800 files for instant hackathon demonstration...")
        try:
            from backend.data_gen import generate_mock_dataset
            res = generate_mock_dataset(TEST_DATASET_DIR, 800)
            print(f"[+] Created test dataset with {res['files_created']} files across {res['subdirectories']} folders.")
        except Exception as e:
            print(f"[!] Could not pre-generate dataset: {e}")
    else:
        print("[+] Test dataset ready.")

def main():
    print("=" * 65)
    print("   AeroSearch: High-Performance Parallel File Search Engine")
    print("       C Core (POSIX Threads) + FastAPI + Modern Web UI")
    print("=" * 65)

    check_or_compile_c()
    check_test_dataset()

    port = 8000
    url = f"http://127.0.0.1:{port}/"

    print(f"\n[+] Starting AeroSearch Web Server at: {url}")
    print("[+] Press Ctrl+C to stop.\n")

    # Open browser after a brief delay
    def open_browser():
        time.sleep(1.2)
        try:
            webbrowser.open(url)
        except Exception:
            pass

    import threading
    threading.Thread(target=open_browser, daemon=True).start()

    # Start FastAPI server
    uvicorn.run("backend.main:app", host="127.0.0.1", port=port, log_level="info")

if __name__ == "__main__":
    main()
