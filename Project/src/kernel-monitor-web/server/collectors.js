// Bridge only: CPU, memory and process collection now happens in C.
const { spawn } = require("node:child_process");
const path = require("node:path");
const readline = require("node:readline");

let child = null;
let latest = null;
let receivedAt = 0;
let failure = "Waiting for the first C collector sample";

function startCollector(onSample, onFatal) {
  if (process.platform !== "linux") {
    throw new Error("The C collector requires Linux. Run both servers inside Ubuntu/WSL2.");
  }
  const executable = path.join(__dirname, "..", "collector", "kernel_collector");
  child = spawn(executable, [], { stdio: ["ignore", "pipe", "inherit"] });
  const lines = readline.createInterface({ input: child.stdout });
  let failed = false;
  function reportFailure(message) {
    if (failed) return;
    failed = true;
    failure = message;
    latest = null;
    onFatal(new Error(message));
  }
  lines.on("line", (line) => {
    if (failed) return;
    try {
      const sample = JSON.parse(line);
      if (sample.collector !== "c" || !Number.isFinite(sample.cpuPercent) ||
          !Number.isFinite(sample.memory?.usedPercent) || !Array.isArray(sample.processes)) {
        throw new Error("Unexpected collector JSON format");
      }
      latest = sample;
      receivedAt = performance.now();
      failure = null;
      onSample(sample);
    } catch (error) {
      reportFailure(`C collector sample failed: ${error.message}`);
    }
  });
  child.on("error", (error) => reportFailure(
    `Cannot start C collector: ${error.message}. Run 'make -C collector' from the project root.`
  ));
  child.on("exit", (code, signal) => {
    reportFailure(`C collector exited (code=${code}, signal=${signal})`);
  });
}

function getSnapshot() {
  if (!latest || performance.now() - receivedAt > 5000) return null;
  return latest;
}
function getCollectorError() {
  return failure || "C collector has not produced a sample within five seconds";
}
function stopCollector() {
  if (child && child.exitCode === null) child.kill("SIGTERM");
}
module.exports = { startCollector, getSnapshot, getCollectorError, stopCollector };
