export default function StatCard({ label, value, unit, tone = "nominal", sub }) {
  const color = `var(--${tone})`;
  return (
    <div className="panel" style={{ padding: "16px 18px", flex: 1, minWidth: 0, position: "relative", overflow: "hidden" }}>
      <div style={{ position: "absolute", top: 0, left: 0, right: 0, height: 2, background: color, opacity: 0.7 }} />
      <div style={{ fontSize: 11.5, color: "var(--text-dim)", marginBottom: 8, letterSpacing: 0.2 }}>{label}</div>
      <div style={{ display: "flex", alignItems: "baseline", gap: 4 }}>
        <span className="mono" style={{ fontSize: 30, fontWeight: 600, color }}>
          {value}
        </span>
        {unit && (
          <span className="mono" style={{ fontSize: 13, color: "var(--text-faint)" }}>
            {unit}
          </span>
        )}
      </div>
      {sub && <div style={{ fontSize: 11, color: "var(--text-faint)", marginTop: 6 }}>{sub}</div>}
    </div>
  );
}
