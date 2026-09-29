import { useEffect, useState } from "react";
import ThemeToggle from "./ThemeToggle";

function formatUptime(seconds) {
  if (!seconds && seconds !== 0) return "--";
  const d = Math.floor(seconds / 86400);
  const h = Math.floor((seconds % 86400) / 3600);
  const m = Math.floor((seconds % 3600) / 60);
  return `${d}d ${h}h ${m}m`;
}

export default function Header({ system, connected, healthScore, theme, onThemeChange }) {
  const [now, setNow] = useState(new Date());
  useEffect(() => {
    const t = setInterval(() => setNow(new Date()), 1000);
    return () => clearInterval(t);
  }, []);

  const scoreColor =
    healthScore == null ? "var(--text-dim)" : healthScore >= 70 ? "var(--nominal)" : healthScore >= 40 ? "var(--warn)" : "var(--critical)";

  return (
    <header
      style={{
        display: "flex",
        alignItems: "center",
        gap: 24,
        padding: "14px 24px",
        borderBottom: "1px solid var(--border)",
        background: "var(--panel)",
      }}
    >
      <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
        <span
          style={{
            width: 8,
            height: 8,
            borderRadius: "50%",
            background: connected ? "var(--nominal)" : "var(--critical)",
            boxShadow: connected ? "0 0 0 3px var(--nominal-dim)" : "0 0 0 3px var(--critical-dim)",
          }}
        />
        <span className="mono" style={{ fontSize: 12, color: connected ? "var(--nominal)" : "var(--critical)" }}>
          {connected ? "LIVE" : "DISCONNECTED"}
        </span>
      </div>

      {system && (
        <div className="mono" style={{ fontSize: 12, color: "var(--text-dim)", display: "flex", gap: 18 }}>
          <span>{system.hostname}</span>
          <span>{system.cpuCores} core{system.cpuCores === 1 ? "" : "s"}</span>
          <span>up {formatUptime(system.uptimeSeconds)}</span>
          <span style={{ color: system.usingRealProc ? "var(--nominal)" : "var(--warn)" }}>
            {system.usingRealProc ? "/proc: live" : "/proc: simulated"}
          </span>
        </div>
      )}

      <div style={{ flex: 1 }} />

      <div style={{ display: "flex", alignItems: "center", gap: 10 }}>
        <span style={{ fontSize: 11, color: "var(--text-faint)" }}>SYSTEM HEALTH</span>
        <span className="mono" style={{ fontSize: 16, fontWeight: 600, color: scoreColor }}>
          {healthScore == null ? "--" : Math.round(healthScore)}
        </span>
      </div>

      <div className="mono" style={{ fontSize: 12, color: "var(--text-faint)" }}>
        {now.toLocaleTimeString()}
      </div>

      <ThemeToggle theme={theme} onChange={onThemeChange} />
    </header>
  );
}
