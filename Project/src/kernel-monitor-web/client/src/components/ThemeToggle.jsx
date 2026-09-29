import { Sun, Moon, MonitorSmartphone } from "lucide-react";

const OPTIONS = [
  { id: "light", icon: Sun, label: "Light" },
  { id: "dark", icon: Moon, label: "Dark" },
  { id: "system", icon: MonitorSmartphone, label: "System" },
];

export default function ThemeToggle({ theme, onChange }) {
  return (
    <div
      style={{
        display: "flex",
        border: "1px solid var(--border)",
        borderRadius: "var(--radius-sm)",
        overflow: "hidden",
        background: "var(--panel-raised)",
      }}
    >
      {OPTIONS.map(({ id, icon: Icon, label }) => {
        const active = theme === id;
        return (
          <button
            key={id}
            onClick={() => onChange(id)}
            title={label}
            aria-label={label}
            style={{
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              width: 30,
              height: 30,
              border: "none",
              borderLeft: id !== "light" ? "1px solid var(--border)" : "none",
              background: active ? "var(--nominal-dim)" : "transparent",
              color: active ? "var(--nominal)" : "var(--text-faint)",
              cursor: "pointer",
              transition: "all 0.15s ease",
            }}
          >
            <Icon size={14} />
          </button>
        );
      })}
    </div>
  );
}
