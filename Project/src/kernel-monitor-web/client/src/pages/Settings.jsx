import { useState } from "react";
import { Volume2, VolumeX, Rows3, Rows4 } from "lucide-react";
import { api } from "../api";
import ThemeToggle from "../components/ThemeToggle";

function Slider({ label, value, onChange, color, min = 10, max = 99, suffix = "%" }) {
  return (
    <div style={{ marginBottom: 18 }}>
      <div style={{ display: "flex", justifyContent: "space-between", marginBottom: 6 }}>
        <span style={{ fontSize: 13 }}>{label}</span>
        <span className="mono" style={{ fontSize: 13, color }}>
          {value}
          {suffix}
        </span>
      </div>
      <input type="range" min={min} max={max} value={value} onChange={(e) => onChange(Number(e.target.value))} style={{ width: "100%" }} />
    </div>
  );
}

function SectionCard({ title, description, children }) {
  return (
    <div className="panel" style={{ padding: 18 }}>
      <div style={{ fontSize: 13, fontWeight: 600, marginBottom: description ? 4 : 14 }}>{title}</div>
      {description && <div style={{ fontSize: 11.5, color: "var(--text-faint)", marginBottom: 14 }}>{description}</div>}
      {children}
    </div>
  );
}

function Row({ children }) {
  return (
    <div style={{ display: "flex", alignItems: "center", justifyContent: "space-between", padding: "10px 0", borderTop: "1px solid var(--border)" }}>
      {children}
    </div>
  );
}

export default function Settings({
  thresholds,
  onSaved,
  theme,
  onThemeChange,
  density,
  onDensityChange,
  refreshMs,
  onRefreshChange,
  soundEnabled,
  onSoundChange,
}) {
  const [cpu, setCpu] = useState(thresholds.cpuPercent);
  const [mem, setMem] = useState(thresholds.memPercent);
  const [status, setStatus] = useState("");

  const flash = (msg) => {
    setStatus(msg);
    setTimeout(() => setStatus(""), 1600);
  };

  const save = async () => {
    try {
      const result = await api.setThresholds({ cpuPercent: cpu, memPercent: mem });
      onSaved(result);
      flash("Thresholds saved");
    } catch {
      flash("Failed to reach backend");
    }
  };

  const clearPins = () => {
    localStorage.removeItem("km_pinned");
    flash("Pinned processes cleared");
  };

  const soundOn = soundEnabled === "true";

  return (
    <div style={{ maxWidth: 520, display: "flex", flexDirection: "column", gap: 18 }}>
      <SectionCard title="Appearance" description="Pick a theme, or follow your OS setting.">
        <Row>
          <span style={{ fontSize: 13 }}>Theme</span>
          <ThemeToggle theme={theme} onChange={onThemeChange} />
        </Row>
        <Row>
          <span style={{ fontSize: 13 }}>Table density</span>
          <div
            style={{
              display: "flex",
              border: "1px solid var(--border)",
              borderRadius: "var(--radius-sm)",
              overflow: "hidden",
              background: "var(--panel-raised)",
            }}
          >
            {[
              { id: "comfortable", icon: Rows4, label: "Comfortable" },
              { id: "compact", icon: Rows3, label: "Compact" },
            ].map(({ id, icon: Icon, label }) => (
              <button
                key={id}
                onClick={() => onDensityChange(id)}
                title={label}
                style={{
                  display: "flex",
                  alignItems: "center",
                  gap: 6,
                  padding: "6px 12px",
                  border: "none",
                  borderLeft: id === "compact" ? "1px solid var(--border)" : "none",
                  background: density === id ? "var(--nominal-dim)" : "transparent",
                  color: density === id ? "var(--nominal)" : "var(--text-faint)",
                  cursor: "pointer",
                  fontSize: 12,
                }}
              >
                <Icon size={13} /> {label}
              </button>
            ))}
          </div>
        </Row>
      </SectionCard>

      <SectionCard title="Live updates" description="Control how often the dashboard polls the backend, and whether alerts make a sound.">
        <Slider
          label="Refresh interval"
          value={Number(refreshMs)}
          onChange={(v) => onRefreshChange(String(v))}
          color="var(--info)"
          min={500}
          max={5000}
          suffix=" ms"
        />
        <Row>
          <span style={{ fontSize: 13 }}>Alert sound</span>
          <button className="icon-btn" onClick={() => onSoundChange(soundOn ? "false" : "true")} title={soundOn ? "Mute alerts" : "Unmute alerts"}>
            {soundOn ? <Volume2 size={15} color="var(--nominal)" /> : <VolumeX size={15} />}
          </button>
        </Row>
      </SectionCard>

      <SectionCard title="Alert thresholds" description="A sample crossing either line raises an alert on the Alerts tab.">
        <Slider label="CPU usage" value={cpu} onChange={setCpu} color="var(--nominal)" />
        <Slider label="Memory usage" value={mem} onChange={setMem} color="var(--info)" />
        <button onClick={save} className="btn btn-primary">
          Save thresholds
        </button>
      </SectionCard>

      <SectionCard title="Backend connection">
        <div className="mono" style={{ fontSize: 12, color: "var(--text-dim)" }}>
          {api.baseUrl}
        </div>
        <div style={{ fontSize: 11.5, color: "var(--text-faint)", marginTop: 8 }}>
          Override with a <code className="mono">VITE_API_URL</code> environment variable if the backend runs on a different host or port.
        </div>
      </SectionCard>

      <SectionCard title="Pinned processes" description="Pinned rows are kept at the top of the Processes table across sessions.">
        <button onClick={clearPins} className="btn">
          Clear all pins
        </button>
      </SectionCard>

      {status && <div style={{ fontSize: 12, color: "var(--nominal)" }}>{status}</div>}
    </div>
  );
}
