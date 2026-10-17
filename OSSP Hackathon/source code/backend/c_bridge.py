import asyncio
import json
import os
import sys
import subprocess
import time
from typing import AsyncGenerator, Dict, Any, Optional

WORKSPACE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
C_ENGINE_DIR = os.path.join(WORKSPACE_DIR, "c_engine")
C_EXE_PATH = os.path.join(C_ENGINE_DIR, "parallel_search.exe")
C_ELF_PATH = os.path.join(C_ENGINE_DIR, "parallel_search")

def to_wsl_path(win_path: str) -> str:
    """Convert Windows path (e.g. E:\folder\file) to WSL path (/mnt/e/folder/file)"""
    abs_path = os.path.abspath(win_path).replace("\\", "/")
    if len(abs_path) >= 2 and abs_path[1] == ":":
        drive = abs_path[0].lower()
        rest = abs_path[2:]
        return f"/mnt/{drive}{rest}"
    return abs_path

def from_wsl_path(wsl_path: str) -> str:
    """Convert WSL path (/mnt/e/...) back to Windows path (E:/...)"""
    if wsl_path.startswith("/mnt/"):
        parts = wsl_path.split("/")
        if len(parts) >= 3 and len(parts[2]) == 1:
            drive = parts[2].upper()
            rest = "/".join(parts[3:])
            return f"{drive}:/{rest}"
    return wsl_path

def get_engine_mode() -> str:
    if os.path.exists(C_EXE_PATH):
        return "native_c"
    elif os.path.exists(C_ELF_PATH):
        return "wsl_c"
    return "python_fallback"

async def run_c_search_stream(
    directory: str,
    pattern: str,
    num_threads: int = 4,
    search_name: bool = True,
    search_content: bool = True,
    case_sensitive: bool = False,
    extensions: str = "",
    max_depth: int = 0
) -> AsyncGenerator[Dict[str, Any], None]:
    """
    Spawns the C parallel search engine and yields streaming JSON events.
    """
    mode = get_engine_mode()
    cmd = []
    use_wsl = False

    if mode == "native_c":
        cmd = [C_EXE_PATH]
        search_dir = os.path.abspath(directory)
    elif mode == "wsl_c":
        use_wsl = True
        c_elf_wsl = to_wsl_path(C_ELF_PATH)
        search_dir = to_wsl_path(directory)
        cmd = ["wsl", c_elf_wsl]
    else:
        # Fallback to pure python multi-threaded search if binary not yet compiled
        async for event in run_python_fallback_stream(
            directory, pattern, num_threads, search_name, search_content, case_sensitive, extensions, max_depth
        ):
            yield event
        return

    # Add CLI options to C engine command
    cmd.extend(["-d", search_dir])
    if pattern:
        cmd.extend(["-p", pattern])
    cmd.extend(["-t", str(num_threads)])
    cmd.append("--stream")

    if search_name and not search_content:
        cmd.append("--name")
    elif search_content and not search_name:
        cmd.append("--content")
    # If both, the C engine defaults to searching both

    if case_sensitive:
        cmd.append("--case-sensitive")
    if extensions:
        cmd.extend(["--ext", extensions])
    if max_depth > 0:
        cmd.extend(["--max-depth", str(max_depth)])

    # Spawn asynchronous subprocess
    try:
        proc = await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
    except Exception as e:
        yield {"type": "error", "message": f"Failed to launch C search engine: {str(e)}"}
        return

    # Stream lines from C stdout
    while True:
        line_bytes = await proc.stdout.readline()
        if not line_bytes:
            break
        line_str = line_bytes.decode("utf-8", errors="replace").strip()
        if not line_str:
            continue

        try:
            event = json.loads(line_str)
            # Reformat paths for display if WSL was used
            if use_wsl:
                if "path" in event:
                    event["path"] = from_wsl_path(event["path"])
                if "threads" in event:
                    for t in event["threads"]:
                        if "current_file" in t and t["current_file"]:
                            t["current_file"] = from_wsl_path(t["current_file"])

            yield event
        except json.JSONDecodeError:
            # Non-JSON debugging log from C
            yield {"type": "log", "message": line_str}

    await proc.wait()


# Python fallback in case C binary is ever missing
async def run_python_fallback_stream(
    directory: str,
    pattern: str,
    num_threads: int = 4,
    search_name: bool = True,
    search_content: bool = True,
    case_sensitive: bool = False,
    extensions: str = "",
    max_depth: int = 0
) -> AsyncGenerator[Dict[str, Any], None]:
    from concurrent.futures import ThreadPoolExecutor
    import queue
    import re

    start_time = time.time()
    ext_list = [e.strip().lower() for e in extensions.split(",") if e.strip()]
    if ext_list:
        ext_list = [e if e.startswith(".") else f".{e}" for e in ext_list]

    total_files = 0
    total_dirs = 0
    total_matches = 0

    # Collect files
    file_queue = []
    for root, dirs, files in os.walk(directory):
        if any(ignored in root for ignored in [".git", "node_modules", "__pycache__", ".vscode"]):
            continue
        total_dirs += len(dirs)
        for f in files:
            full_path = os.path.join(root, f)
            if ext_list and not any(f.lower().endswith(ext) for ext in ext_list):
                continue
            file_queue.append(full_path)

    total_files = len(file_queue)

    flags = 0 if case_sensitive else re.IGNORECASE
    compiled_pattern = None
    if pattern:
        try:
            compiled_pattern = re.compile(re.escape(pattern), flags)
        except Exception:
            compiled_pattern = None

    # Process files in parallel
    chunk_size = max(1, len(file_queue) // (num_threads * 4))
    
    for i, file_path in enumerate(file_queue):
        filename = os.path.basename(file_path)
        tid = i % num_threads

        # Filename check
        if search_name and compiled_pattern and compiled_pattern.search(filename):
            total_matches += 1
            yield {
                "type": "match",
                "thread_id": tid,
                "path": file_path.replace("\\", "/"),
                "line": 0,
                "snippet": f"Filename matched: {filename}",
                "match_type": "name"
            }

        # Content check
        if search_content and pattern:
            try:
                with open(file_path, "r", encoding="utf-8", errors="ignore") as fp:
                    for line_idx, line in enumerate(fp, start=1):
                        if compiled_pattern and compiled_pattern.search(line):
                            total_matches += 1
                            yield {
                                "type": "match",
                                "thread_id": tid,
                                "path": file_path.replace("\\", "/"),
                                "line": line_idx,
                                "snippet": line.strip()[:180],
                                "match_type": "content"
                            }
            except Exception:
                pass

        if i % 25 == 0:
            elapsed_ms = (time.time() - start_time) * 1000.0
            yield {
                "type": "telemetry",
                "elapsed_ms": elapsed_ms,
                "total_files": total_files,
                "total_dirs": total_dirs,
                "total_matches": total_matches,
                "threads": [
                    {
                        "id": t,
                        "status": "SEARCHING",
                        "files": (i // num_threads),
                        "matches": (total_matches // num_threads),
                        "current_file": file_path.replace("\\", "/")
                    }
                    for t in range(num_threads)
                ]
            }
            await asyncio.sleep(0.005)

    duration_ms = (time.time() - start_time) * 1000.0
    yield {
        "type": "done",
        "threads": num_threads,
        "total_files": total_files,
        "total_dirs": total_dirs,
        "total_matches": total_matches,
        "duration_ms": duration_ms
    }
