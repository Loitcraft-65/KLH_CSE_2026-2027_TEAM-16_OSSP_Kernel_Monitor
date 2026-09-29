import { AlertTriangle, Cpu, MemoryStick } from "lucide-react";

export default function Alerts({ alerts }) {
  if (!alerts) return null;

  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
      <div style={{ fontSize: 12, color: "var(--text-faint)" }}>
        Alerts fire when a sample crosses your configured thresholds (see Settings). Repeated breaches of the
        same kind are debounced to one entry per 10 seconds so the feed stays readable.
      </div>

      {alerts.length === 0 ? (
        <div className="panel" style={{ padding: 32, textAlign: "center", color: "var(--text-faint)" }}>
          No alerts yet — the system is within its configured thresholds.
        </div>
      ) : (
        <div style={{ display: "flex", flexDirection: "column", gap: 8 }}>
          {[...alerts].reverse().map((a) => (
            <div
              key={a.id}
              className="panel"
              style={{
                display: "flex",
                alignItems: "center",
                gap: 12,
                borderLeft: "3px solid var(--critical)",
                padding: "10px 14px",
              }}
            >
              {a.type === "cpu" ? <Cpu size={16} color="var(--critical)" /> : <MemoryStick size={16} color="var(--critical)" />}
              <div style={{ flex: 1 }}>
                <div style={{ fontSize: 13 }}>{a.message}</div>
                <div className="mono" style={{ fontSize: 11, color: "var(--text-faint)" }}>
                  {new Date(a.t).toLocaleString()}
                </div>
              </div>
              <AlertTriangle size={14} color="var(--warn)" />
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
