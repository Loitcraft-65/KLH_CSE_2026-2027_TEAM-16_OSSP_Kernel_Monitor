"""
Kernel Monitor - FastAPI REST API & WebSocket Server
Lead: Saahas Vaddi (Roll: 2520030256)
"""

import asyncio
import os
from contextlib import asynccontextmanager
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse

from .config import thresholds, ThresholdConfig, LOG_FILE_PATH
from .database import init_db, get_recent_history, get_recent_alerts
from .collector_bridge import (
    run_collector_bridge, stop_collector_bridge,
    get_latest_snapshot, snapshot_listeners, handle_new_snapshot
)
from .alerting import recent_active_alerts

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Initialize SQLite database
    await init_db()
    # Start the C Collector bridge
    bridge_task = asyncio.create_task(run_collector_bridge())
    yield
    # Cleanup
    stop_collector_bridge()
    bridge_task.cancel()

app = FastAPI(
    title="Linux Kernel Monitor API",
    description="OSSP (25CS2104E) - Kernel Subsystems & Performance Telemetry",
    version="1.0.0",
    lifespan=lifespan
)

# Enable CORS for frontend flexibility
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/api/status")
async def get_status():
    snap = get_latest_snapshot()
    return {
        "status": "online",
        "service": "Linux Kernel Subsystems Monitor",
        "has_live_telemetry": bool(snap),
        "kernel_version": snap.get("kernel_version", "Linux Kernel")
    }

@app.get("/api/metrics/live")
async def get_live_metrics():
    snap = get_latest_snapshot()
    if not snap:
        raise HTTPException(status_code=503, detail="Collector initializing...")
    return snap

@app.get("/api/metrics/history")
async def get_history(limit: int = 60):
    return await get_recent_history(limit=limit)

@app.get("/api/alerts")
async def get_alerts():
    return {
        "active_alerts": recent_active_alerts,
        "recent_history": await get_recent_alerts(limit=50)
    }

@app.get("/api/thresholds")
async def get_thresholds():
    return thresholds.model_dump()

@app.post("/api/thresholds")
async def update_thresholds(new_cfg: ThresholdConfig):
    global thresholds
    thresholds = new_cfg
    return {"message": "Thresholds updated successfully", "thresholds": thresholds.model_dump()}

@app.get("/api/logs")
async def get_logs(lines: int = 40):
    if not os.path.exists(LOG_FILE_PATH):
        return {"logs": ["No logs recorded yet."]}
    try:
        with open(LOG_FILE_PATH, "r", encoding="utf-8") as f:
            all_lines = f.readlines()
            return {"logs": all_lines[-lines:]}
    except Exception as e:
        return {"logs": [f"Error reading log file: {e}"]}

@app.post("/api/ingest")
async def ingest_snapshot(data: dict):
    """Direct HTTP ingest endpoint for collector metrics."""
    await handle_new_snapshot(data)
    return {"status": "ok"}

@app.websocket("/ws/live")
async def websocket_live_stream(websocket: WebSocket):
    await websocket.accept()
    queue = asyncio.Queue()
    snapshot_listeners.append(queue)
    try:
        # Send current snapshot immediately on connection
        snap = get_latest_snapshot()
        if snap:
            await websocket.send_json(snap)

        while True:
            data = await queue.get()
            await websocket.send_json(data)
    except (WebSocketDisconnect, asyncio.CancelledError):
        pass
    finally:
        if queue in snapshot_listeners:
            snapshot_listeners.remove(queue)

# Serve Frontend static assets
frontend_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "frontend"))
if os.path.exists(frontend_dir):
    app.mount("/static", StaticFiles(directory=frontend_dir), name="static")

    @app.get("/")
    async def serve_index():
        return FileResponse(os.path.join(frontend_dir, "index.html"))
