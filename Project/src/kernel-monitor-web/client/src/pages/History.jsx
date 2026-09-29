import { Download } from "lucide-react";
import TrendChart from "../components/TrendChart";
import { api } from "../api";

export default function History({ history, thresholds }) {
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 16 }}>
      <div style={{ display: "flex", alignItems: "center", justifyContent: "space-between" }}>
        <div>
          <div style={{ fontSize: 13, fontWeight: 600 }}>Resource history log</div>
          <div style={{ fontSize: 11.5, color: "var(--text-faint)" }}>
            Buffers the last 15 minutes at 1-second resolution — mirroring the log file the C collector writes to disk.
          </div>
        </div>
        <div style={{ display: "flex", gap: 8 }}>
          <a href={api.exportCsvUrl()} className="btn" style={{ textDecoration: "none" }} download>
            <Download size={13} /> CSV
          </a>
          <a href={api.exportJsonUrl()} className="btn" style={{ textDecoration: "none" }} download>
            <Download size={13} /> JSON
          </a>
        </div>
      </div>

      <div className="panel" style={{ padding: 16 }}>
        <div style={{ fontSize: 12, color: "var(--text-dim)", marginBottom: 4 }}>CPU %</div>
        <TrendChart data={history} dataKey="cpu" color="var(--nominal)" threshold={thresholds.cpuPercent} height={200} />
      </div>
      <div className="panel" style={{ padding: 16 }}>
        <div style={{ fontSize: 12, color: "var(--text-dim)", marginBottom: 4 }}>Memory %</div>
        <TrendChart data={history} dataKey="mem" color="var(--info)" threshold={thresholds.memPercent} height={200} />
      </div>
    </div>
  );
}

