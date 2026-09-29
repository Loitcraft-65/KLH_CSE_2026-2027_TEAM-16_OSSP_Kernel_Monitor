import { useState } from "react";
import { Camera, X } from "lucide-react";
import StatCard from "../components/StatCard";
import TrendChart from "../components/TrendChart";
import HealthGauge from "../components/HealthGauge";

function bytesToGb(b) {
  return (b / 1024 ** 3).toFixed(2);
}

function toneFor(value, threshold) {
  if (value >= threshold) return "critical";
  if (value >= threshold * 0.75) return "warn";
  return "nominal";
}

export default function Overview({ snapshot, history, thresholds, healthScore }) {
  const [snap, setSnap] = useState(null);

  if (!snapshot) return <div style={{ color: "var(--text-dim)" }}>Waiting for data…</div>;

  const { cpuPercent, memory, processes } = snapshot;
  const totalThreads = processes.reduce((a, p) => a + (p.threads || 0), 0);

  const takeSnapshot = () =>
    setSnap({ t: Date.now(), cpuPercent, memUsedPercent: memory.usedPercent, processCount: processes.length, totalThreads });

  const delta = (curr, prev) => {
    const d = curr - prev;
    const sign = d > 0 ? "+" : "";
    return `${sign}${d.toFixed(1)}`;
  };

  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 18 }}>
      <div style={{ display: "flex", gap: 14 }}>
        <StatCard
          label="CPU USAGE"
          value={cpuPercent.toFixed(1)}
          unit="%"
          tone={toneFor(cpuPercent, thresholds.cpuPercent)}
          sub={`threshold ${thresholds.cpuPercent}%`}
        />
        <StatCard
          label="MEMORY USAGE"
          value={memory.usedPercent.toFixed(1)}
          unit="%"
          tone={toneFor(memory.usedPercent, thresholds.memPercent)}
          sub={`${bytesToGb(memory.usedBytes)} / ${bytesToGb(memory.totalBytes)} GB`}
        />
        <StatCard label="PROCESSES" value={processes.length} tone="nominal" sub={`${totalThreads} threads`} />
        <div className="panel" style={{ padding: "10px 18px", display: "flex", alignItems: "center", justifyContent: "center" }}>
          <HealthGauge score={healthScore} size={120} />
        </div>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 14 }}>
        <div className="panel" style={{ padding: 16 }}>
          <div style={{ fontSize: 12, color: "var(--text-dim)", marginBottom: 4 }}>CPU · last 15 min</div>
          <TrendChart data={history} dataKey="cpu" color="var(--nominal)" threshold={thresholds.cpuPercent} />
        </div>
        <div className="panel" style={{ padding: 16 }}>
          <div style={{ fontSize: 12, color: "var(--text-dim)", marginBottom: 4 }}>Memory · last 15 min</div>
          <TrendChart data={history} dataKey="mem" color="var(--info)" threshold={thresholds.memPercent} />
        </div>
      </div>

      <div className="panel" style={{ padding: 16 }}>
        <div style={{ display: "flex", alignItems: "center", justifyContent: "space-between", marginBottom: 10 }}>
          <div>
            <div style={{ fontSize: 13, fontWeight: 600 }}>Snapshot compare</div>
            <div style={{ fontSize: 11.5, color: "var(--text-faint)" }}>
              Pin a moment in time, then watch how far the system has drifted since.
            </div>
          </div>
          <div style={{ display: "flex", gap: 8 }}>
            {snap && (
              <button onClick={() => setSnap(null)} className="btn">
                <X size={13} /> clear
              </button>
            )}
            <button onClick={takeSnapshot} className="btn btn-primary">
              <Camera size={13} /> take snapshot
            </button>
          </div>
        </div>

        {snap ? (
          <div className="mono" style={{ display: "flex", gap: 28, fontSize: 13 }}>
            <div>
              <div style={{ color: "var(--text-faint)", fontSize: 11 }}>captured</div>
              {new Date(snap.t).toLocaleTimeString()}
            </div>
            <div>
              <div style={{ color: "var(--text-faint)", fontSize: 11 }}>CPU Δ</div>
              {delta(cpuPercent, snap.cpuPercent)}%
            </div>
            <div>
              <div style={{ color: "var(--text-faint)", fontSize: 11 }}>MEM Δ</div>
              {delta(memory.usedPercent, snap.memUsedPercent)}%
            </div>
            <div>
              <div style={{ color: "var(--text-faint)", fontSize: 11 }}>PROC Δ</div>
              {delta(processes.length, snap.processCount)}
            </div>
            <div>
              <div style={{ color: "var(--text-faint)", fontSize: 11 }}>THREADS Δ</div>
              {delta(totalThreads, snap.totalThreads)}
            </div>
          </div>
        ) : (
          <div style={{ fontSize: 12, color: "var(--text-faint)" }}>No snapshot saved yet.</div>
        )}
      </div>
    </div>
  );
}

