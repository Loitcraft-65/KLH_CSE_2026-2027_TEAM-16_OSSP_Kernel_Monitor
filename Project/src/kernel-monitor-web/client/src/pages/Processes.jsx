import { useEffect, useMemo, useState } from "react";
import { Star, Search } from "lucide-react";

const STATE_LABELS = { R: "Running", S: "Sleeping", D: "Disk wait", Z: "Zombie", T: "Stopped", I: "Idle" };

function loadPinned() {
  try {
    return new Set(JSON.parse(localStorage.getItem("km_pinned") || "[]"));
  } catch {
    return new Set();
  }
}

export default function Processes({ snapshot }) {
  const [query, setQuery] = useState("");
  const [sortKey, setSortKey] = useState("cpuPercent");
  const [sortDir, setSortDir] = useState("desc");
  const [pinned, setPinned] = useState(loadPinned);

  useEffect(() => {
    localStorage.setItem("km_pinned", JSON.stringify([...pinned]));
  }, [pinned]);

  const togglePin = (pid) =>
    setPinned((prev) => {
      const next = new Set(prev);
      if (next.has(pid)) next.delete(pid);
      else next.add(pid);
      return next;
    });

  const rows = useMemo(() => {
    if (!snapshot) return [];
    let list = snapshot.processes.filter(
      (p) => !query || p.name.toLowerCase().includes(query.toLowerCase()) || String(p.pid).includes(query)
    );
    list = [...list].sort((a, b) => {
      const dir = sortDir === "asc" ? 1 : -1;
      if (a[sortKey] < b[sortKey]) return -1 * dir;
      if (a[sortKey] > b[sortKey]) return 1 * dir;
      return 0;
    });
    list.sort((a, b) => (pinned.has(b.pid) ? 1 : 0) - (pinned.has(a.pid) ? 1 : 0));
    return list;
  }, [snapshot, query, sortKey, sortDir, pinned]);

  const setSort = (key) => {
    if (sortKey === key) setSortDir((d) => (d === "asc" ? "desc" : "asc"));
    else {
      setSortKey(key);
      setSortDir("desc");
    }
  };

  const col = (key, label) => (
    <th
      onClick={() => setSort(key)}
      style={{ cursor: "pointer", textAlign: "left", padding: "8px 10px", color: sortKey === key ? "var(--nominal)" : "var(--text-dim)" }}
    >
      {label} {sortKey === key ? (sortDir === "asc" ? "↑" : "↓") : ""}
    </th>
  );

  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 12 }}>
      <div style={{ display: "flex", alignItems: "center", gap: 10 }}>
        <div style={{ position: "relative", flex: 1, maxWidth: 340 }}>
          <Search size={14} style={{ position: "absolute", left: 10, top: 9, color: "var(--text-faint)" }} />
          <input
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            placeholder="Search by name or PID…"
            style={{
              width: "100%",
              padding: "8px 10px 8px 30px",
              background: "var(--panel)",
              border: "1px solid var(--border)",
              borderRadius: "var(--radius)",
              color: "var(--text)",
              fontSize: 12.5,
            }}
          />
        </div>
        <span style={{ fontSize: 11.5, color: "var(--text-faint)" }}>
          {rows.length} process{rows.length === 1 ? "" : "es"} · {pinned.size} pinned
        </span>
      </div>

      <div className="panel" style={{ overflow: "hidden" }}>
        <table className="mono" style={{ width: "100%", borderCollapse: "collapse", fontSize: 12.5 }}>
          <thead style={{ borderBottom: "1px solid var(--border)" }}>
            <tr>
              <th style={{ width: 34 }}></th>
              {col("pid", "PID")}
              {col("name", "NAME")}
              {col("state", "STATE")}
              {col("cpuPercent", "CPU %")}
              {col("memMb", "MEM MB")}
              {col("threads", "THREADS")}
            </tr>
          </thead>
          <tbody>
            {rows.map((p) => (
              <tr
                key={p.pid}
                className="km-row"
                style={{
                  borderBottom: "1px solid var(--border)",
                  background: pinned.has(p.pid) ? "var(--nominal-dim)" : "transparent",
                }}
              >
                <td style={{ padding: "var(--row-py) 10px" }}>
                  <button
                    onClick={() => togglePin(p.pid)}
                    style={{ background: "none", border: "none", cursor: "pointer", padding: 0, display: "flex" }}
                    aria-label={pinned.has(p.pid) ? "Unpin process" : "Pin process"}
                  >
                    <Star size={14} fill={pinned.has(p.pid) ? "var(--warn)" : "none"} color={pinned.has(p.pid) ? "var(--warn)" : "var(--text-faint)"} />
                  </button>
                </td>
                <td style={{ padding: "var(--row-py) 10px", color: "var(--text-dim)" }}>{p.pid}</td>
                <td style={{ padding: "var(--row-py) 10px" }}>{p.name}</td>
                <td style={{ padding: "var(--row-py) 10px", color: "var(--text-dim)" }}>{STATE_LABELS[p.state] || p.state}</td>
                <td style={{ padding: "var(--row-py) 10px", color: p.cpuPercent > 50 ? "var(--critical)" : "var(--text)" }}>{p.cpuPercent.toFixed(1)}</td>
                <td style={{ padding: "var(--row-py) 10px" }}>{p.memMb.toFixed(1)}</td>
                <td style={{ padding: "var(--row-py) 10px", color: "var(--text-dim)" }}>{p.threads}</td>
              </tr>
            ))}
            {rows.length === 0 && (
              <tr>
                <td colSpan={7} style={{ padding: 24, textAlign: "center", color: "var(--text-faint)" }}>
                  No processes match "{query}".
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
