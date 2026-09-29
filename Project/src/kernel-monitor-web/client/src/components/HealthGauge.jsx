export default function HealthGauge({ score = 0, size = 150 }) {
  const radius = size / 2 - 12;
  const circumference = Math.PI * radius; // half circle
  const clamped = Math.max(0, Math.min(100, score));
  const offset = circumference - (clamped / 100) * circumference;
  const color = clamped >= 70 ? "var(--nominal)" : clamped >= 40 ? "var(--warn)" : "var(--critical)";
  const label = clamped >= 70 ? "Healthy" : clamped >= 40 ? "Elevated load" : "Critical";

  return (
    <div style={{ display: "flex", flexDirection: "column", alignItems: "center" }}>
      <svg width={size} height={size / 2 + 20} viewBox={`0 0 ${size} ${size / 2 + 20}`}>
        <path
          d={`M 12 ${size / 2} A ${radius} ${radius} 0 0 1 ${size - 12} ${size / 2}`}
          fill="none"
          stroke="var(--border)"
          strokeWidth="10"
          strokeLinecap="round"
        />
        <path
          d={`M 12 ${size / 2} A ${radius} ${radius} 0 0 1 ${size - 12} ${size / 2}`}
          fill="none"
          stroke={color}
          strokeWidth="10"
          strokeLinecap="round"
          strokeDasharray={circumference}
          strokeDashoffset={offset}
          style={{ transition: "stroke-dashoffset 0.6s ease" }}
        />
        <text x={size / 2} y={size / 2 - 6} textAnchor="middle" className="mono" fontSize="26" fontWeight="600" fill={color}>
          {Math.round(clamped)}
        </text>
      </svg>
      <div style={{ fontSize: 12, color: "var(--text-dim)", marginTop: -6 }}>{label}</div>
    </div>
  );
}
