import { Activity, ListTree, BellRing, History as HistoryIcon, SlidersHorizontal, Cpu } from "lucide-react";

const NAV = [
  { id: "overview", label: "Overview", icon: Activity },
  { id: "processes", label: "Processes", icon: ListTree },
  { id: "alerts", label: "Alerts", icon: BellRing },
  { id: "history", label: "History", icon: HistoryIcon },
  { id: "settings", label: "Settings", icon: SlidersHorizontal },
];

export default function Sidebar({ active, onChange, alertCount }) {
  return (
    <aside
      style={{
        width: 220,
        flexShrink: 0,
        borderRight: "1px solid var(--border)",
        display: "flex",
        flexDirection: "column",
        padding: "20px 14px",
        gap: 4,
      }}
    >
      <div style={{ display: "flex", alignItems: "center", gap: 10, padding: "0 8px 22px" }}>
        <Cpu size={20} color="var(--nominal)" />
        <div>
          <div style={{ fontWeight: 700, fontSize: 15, letterSpacing: 0.2 }}>Kernel Monitor</div>
          <div className="mono" style={{ fontSize: 10.5, color: "var(--text-faint)" }}>v1.0 · web console</div>
        </div>
      </div>

      {NAV.map(({ id, label, icon: Icon }) => {
        const isActive = active === id;
        return (
          <button
            key={id}
            onClick={() => onChange(id)}
            style={{
              display: "flex",
              alignItems: "center",
              gap: 10,
              width: "100%",
              padding: "9px 10px",
              border: "none",
              borderRadius: "var(--radius)",
              background: isActive ? "var(--panel-raised)" : "transparent",
              color: isActive ? "var(--text)" : "var(--text-dim)",
              borderLeft: isActive ? "2px solid var(--nominal)" : "2px solid transparent",
              cursor: "pointer",
              fontSize: 13.5,
              textAlign: "left",
            }}
          >
            <Icon size={16} />
            <span style={{ flex: 1 }}>{label}</span>
            {id === "alerts" && alertCount > 0 && (
              <span
                className="mono"
                style={{
                  fontSize: 10.5,
                  background: "var(--critical-dim)",
                  color: "var(--critical)",
                  borderRadius: 8,
                  padding: "1px 6px",
                }}
              >
                {alertCount}
              </span>
            )}
          </button>
        );
      })}

      <div style={{ marginTop: "auto", padding: "10px 8px", fontSize: 11, color: "var(--text-faint)", lineHeight: 1.5 }}>
        Team 16 · OSSP 25CS2104E
        <br />
        Saahas · Lohith · Rithwik
      </div>
    </aside>
  );
}
