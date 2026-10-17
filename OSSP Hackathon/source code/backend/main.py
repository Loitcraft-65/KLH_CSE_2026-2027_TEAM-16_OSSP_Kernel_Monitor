import os
import sys
import psutil
import json
import asyncio
from typing import Optional, List
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from .c_bridge import run_c_search_stream, get_engine_mode, WORKSPACE_DIR
from .benchmark import run_benchmark_suite
from .data_gen import generate_mock_dataset

app = FastAPI(title="Parallel File Search Engine", version="2.0.0")

# Enable CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

FRONTEND_DIR = os.path.abspath(os.path.join(WORKSPACE_DIR, "frontend"))
TEST_DATASET_DIR = os.path.abspath(os.path.join(WORKSPACE_DIR, "test_dataset"))

class BenchmarkRequest(BaseModel):
    directory: Optional[str] = None
    pattern: Optional[str] = "main"
    threads: Optional[List[int]] = [1, 2, 4, 8, 16]
    search_name: Optional[bool] = True
    search_content: Optional[bool] = True

class GenerateDataRequest(BaseModel):
    directory: Optional[str] = None
    file_count: Optional[int] = 800

@app.get("/api/system-info")
def get_system_info():
    logical_cores = os.cpu_count() or 4
    try:
        physical_cores = psutil.cpu_count(logical=False) or logical_cores
        total_ram_gb = round(psutil.virtual_memory().total / (1024 ** 3), 1)
    except Exception:
        physical_cores = logical_cores
        total_ram_gb = 16.0

    mode = get_engine_mode()
    mode_descriptions = {
        "native_c": "Native C Binary (Windows x86_64, POSIX Threads)",
        "wsl_c": "Compiled C Binary (Linux WSL, POSIX Threads)",
        "python_fallback": "Python ThreadPoolExecutor (Fallback Engine)"
    }

    return {
        "logical_cores": logical_cores,
        "physical_cores": physical_cores,
        "total_ram_gb": total_ram_gb,
        "platform": sys.platform,
        "engine_mode": mode,
        "engine_description": mode_descriptions.get(mode, mode),
        "default_dir": WORKSPACE_DIR.replace("\\", "/"),
        "test_dataset_dir": TEST_DATASET_DIR.replace("\\", "/"),
        "test_dataset_exists": os.path.exists(TEST_DATASET_DIR)
    }

@app.get("/api/browse")
def browse_directories(path: Optional[str] = None):
    target = path if path and os.path.exists(path) else WORKSPACE_DIR
    target = os.path.abspath(target)
    
    entries = []
    parent = os.path.dirname(target) if target != os.path.dirname(target) else None

    try:
        with os.scandir(target) as it:
            for entry in it:
                if entry.is_dir() and not entry.name.startswith("."):
                    entries.append({
                        "name": entry.name,
                        "path": entry.path.replace("\\", "/"),
                        "is_dir": True
                    })
    except PermissionError:
        raise HTTPException(status_code=403, detail="Permission denied")

    entries.sort(key=lambda x: x["name"].lower())

    return {
        "current_path": target.replace("\\", "/"),
        "parent_path": parent.replace("\\", "/") if parent else None,
        "directories": entries[:50]
    }

@app.post("/api/generate-data")
def generate_data(req: GenerateDataRequest):
    target = req.directory if req.directory else TEST_DATASET_DIR
    count = req.file_count if req.file_count and req.file_count > 0 else 800
    if count > 5000:
        count = 5000  # Safety cap for hackathon demo
    res = generate_mock_dataset(target, count)
    return res

@app.post("/api/benchmark")
async def benchmark(req: BenchmarkRequest):
    target = req.directory if req.directory and os.path.exists(req.directory) else WORKSPACE_DIR
    res = await run_benchmark_suite(
        directory=target,
        pattern=req.pattern or "main",
        thread_counts=req.threads or [1, 2, 4, 8, 16],
        search_name=req.search_name if req.search_name is not None else True,
        search_content=req.search_content if req.search_content is not None else True
    )
    return res

@app.websocket("/ws/search")
async def websocket_search_endpoint(websocket: WebSocket):
    await websocket.accept()
    cancel_event = asyncio.Event()

    try:
        # First message must be search configuration
        init_data = await websocket.receive_text()
        config = json.loads(init_data)

        directory = config.get("directory", WORKSPACE_DIR)
        pattern = config.get("pattern", "")
        num_threads = int(config.get("threads", 4))
        search_name = bool(config.get("search_name", True))
        search_content = bool(config.get("search_content", True))
        case_sensitive = bool(config.get("case_sensitive", False))
        extensions = str(config.get("extensions", ""))
        max_depth = int(config.get("max_depth", 0))

        # Background task to listen for cancellation from client
        async def listen_for_cancel():
            try:
                while True:
                    msg = await websocket.receive_text()
                    try:
                        parsed = json.loads(msg)
                        if parsed.get("action") == "cancel":
                            cancel_event.set()
                            break
                    except Exception:
                        pass
            except Exception:
                cancel_event.set()

        cancel_task = asyncio.create_task(listen_for_cancel())

        # Stream results from C search engine
        async for event in run_c_search_stream(
            directory=directory,
            pattern=pattern,
            num_threads=num_threads,
            search_name=search_name,
            search_content=search_content,
            case_sensitive=case_sensitive,
            extensions=extensions,
            max_depth=max_depth
        ):
            if cancel_event.is_set():
                await websocket.send_text(json.dumps({
                    "type": "cancelled",
                    "message": "Search was cancelled by user."
                }))
                break

            await websocket.send_text(json.dumps(event))

        cancel_task.cancel()

    except WebSocketDisconnect:
        pass
    except Exception as e:
        try:
            await websocket.send_text(json.dumps({"type": "error", "message": str(e)}))
        except Exception:
            pass
    finally:
        try:
            await websocket.close()
        except Exception:
            pass

# Mount frontend static files
if os.path.exists(FRONTEND_DIR):
    app.mount("/", StaticFiles(directory=FRONTEND_DIR, html=True), name="frontend")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("backend.main:app", host="127.0.0.1", port=8000, reload=True)
