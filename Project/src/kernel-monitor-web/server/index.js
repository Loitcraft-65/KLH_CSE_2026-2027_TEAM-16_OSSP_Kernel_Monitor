const express = require("express");
const cors = require("cors");
const os = require("node:os");
const { startCollector, getSnapshot, getCollectorError, stopCollector } = require("./collectors");

const app = express();
app.use(cors());
app.use(express.json());
const PORT = process.env.PORT || 5000;
const HISTORY_LIMIT = 900;
let thresholds = { cpuPercent: 80, memPercent: 85 };
const history = [];
const alerts = [];
const lastAlertTime = new Map();
let alertId = 1;
let stopping = false;
let server;

function pushAlert(type, message, value, t) {
  // A separate cooldown per type handles simultaneous CPU/memory breaches.
  const now = performance.now();
  const last = lastAlertTime.get(type);
  if (last !== undefined && now - last < 10000) return;
  lastAlertTime.set(type, now);
  alerts.push({ id: alertId++, t, type, message, value });
  if (alerts.length > 500) alerts.shift();
}

function sample(pointFromC) {
  const point = { t: pointFromC.t, cpu: pointFromC.cpuPercent, mem: pointFromC.memory.usedPercent };
  history.push(point);
  if (history.length > HISTORY_LIMIT) history.shift();
  if (point.cpu >= thresholds.cpuPercent)
    pushAlert("cpu", `CPU usage ${point.cpu}% reached threshold ${thresholds.cpuPercent}%`, point.cpu, point.t);
  if (point.mem >= thresholds.memPercent)
    pushAlert("memory", `Memory usage ${point.mem}% reached threshold ${thresholds.memPercent}%`, point.mem, point.t);
}

function needSnapshot(res) {
  const snapshot = getSnapshot();
  if (!snapshot) res.status(503).json({ error: getCollectorError() });
  return snapshot;
}
app.get("/api/system", (req, res) => {
  const snapshot = needSnapshot(res);
  if (!snapshot) return;
  // Static host labels use Node's os module; live measurements come from C.
  res.json({ hostname: os.hostname(), platform: os.platform(), arch: os.arch(),
    cpuModel: os.cpus()[0]?.model || "unknown", cpuCores: os.cpus().length,
    uptimeSeconds: snapshot.uptimeSeconds, usingRealProc: true, collector: "c" });
});
app.get("/api/snapshot", (req, res) => {
  const snapshot = needSnapshot(res);
  if (snapshot) res.json(snapshot);
});
app.get("/api/history", (req, res) => res.json(history));
app.get("/api/alerts", (req, res) => res.json(alerts));
app.get("/api/thresholds", (req, res) => res.json(thresholds));
app.post("/api/thresholds", (req, res) => {
  const { cpuPercent, memPercent } = req.body || {};
  const valid = (n) => Number.isFinite(n) && n >= 0 && n <= 100;
  if (!valid(cpuPercent) || !valid(memPercent))
    return res.status(400).json({ error: "Both thresholds must be numbers between 0 and 100" });
  thresholds = { cpuPercent, memPercent };
  res.json(thresholds);
});
app.get("/api/export.json", (req, res) => {
  res.setHeader("Content-Disposition", "attachment; filename=kernel-monitor-log.json");
  res.json({ history, alerts, exportedAt: Date.now() });
});
app.get("/api/export.csv", (req, res) => {
  const rows = ["timestamp,cpu_percent,mem_percent"];
  for (const p of history) rows.push(`${new Date(p.t).toISOString()},${p.cpu},${p.mem}`);
  res.setHeader("Content-Type", "text/csv");
  res.setHeader("Content-Disposition", "attachment; filename=kernel-monitor-log.csv");
  res.send(rows.join("\n"));
});

function shutdown(code = 0) {
  if (stopping) return;
  stopping = true;
  process.exitCode = code;
  stopCollector();
  if (server) server.close();
  // Allow C to handle SIGTERM and close stdout before the parent exits.
  setTimeout(() => process.exit(code), 500).unref();
}
process.on("SIGINT", () => shutdown(0));
process.on("SIGTERM", () => shutdown(0));
process.on("exit", stopCollector);

try {
  startCollector(sample, (error) => {
    if (!stopping) { console.error(error.message); shutdown(1); }
  });
  server = app.listen(PORT, "127.0.0.1", () => {
    console.log(`Kernel Monitor backend: http://localhost:${PORT}`);
    console.log("Reading REAL Linux metrics from the C collector; first sample in about one second.");
  });
  server.on("error", (error) => { console.error(error.message); shutdown(1); });
} catch (error) {
  console.error(error.message);
  shutdown(1);
}
