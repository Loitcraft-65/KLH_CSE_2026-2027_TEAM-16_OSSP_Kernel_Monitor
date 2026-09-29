import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, ReferenceLine } from "recharts";

function formatTime(t) {
  return new Date(t).toLocaleTimeString([], { minute: "2-digit", second: "2-digit" });
}

export default function TrendChart({ data, dataKey, color, threshold, height = 220, unit = "%" }) {
  return (
    <ResponsiveContainer width="100%" height={height}>
      <AreaChart data={data} margin={{ top: 6, right: 12, left: -12, bottom: 0 }}>
        <defs>
          <linearGradient id={`grad-${dataKey}`} x1="0" y1="0" x2="0" y2="1">
            <stop offset="5%" stopColor={color} stopOpacity={0.35} />
            <stop offset="95%" stopColor={color} stopOpacity={0} />
          </linearGradient>
        </defs>
        <CartesianGrid stroke="var(--border)" strokeDasharray="0" vertical={false} />
        <XAxis
          dataKey="t"
          tickFormatter={formatTime}
          stroke="var(--text-faint)"
          fontSize={10.5}
          fontFamily="var(--font-mono)"
          tickLine={false}
          axisLine={{ stroke: "var(--border)" }}
          minTickGap={40}
        />
        <YAxis
          domain={[0, 100]}
          stroke="var(--text-faint)"
          fontSize={10.5}
          fontFamily="var(--font-mono)"
          tickLine={false}
          axisLine={false}
          width={34}
        />
        {threshold != null && (
          <ReferenceLine y={threshold} stroke="var(--critical)" strokeDasharray="4 4" strokeOpacity={0.6} />
        )}
        <Tooltip
          contentStyle={{
            background: "var(--panel-raised)",
            border: "1px solid var(--border)",
            borderRadius: 3,
            fontFamily: "var(--font-mono)",
            fontSize: 12,
          }}
          labelFormatter={formatTime}
          formatter={(v) => [`${v}${unit}`, dataKey]}
        />
        <Area type="monotone" dataKey={dataKey} stroke={color} strokeWidth={1.75} fill={`url(#grad-${dataKey})`} isAnimationActive={false} />
      </AreaChart>
    </ResponsiveContainer>
  );
}
