import { useEffect, useRef, useState } from "react";
import Sidebar from "./components/Sidebar";
import Header from "./components/Header";
import Overview from "./pages/Overview";
import Processes from "./pages/Processes";
import Alerts from "./pages/Alerts";
import History from "./pages/History";
import Settings from "./pages/Settings";
import { usePolling } from "./hooks/usePolling";
import { useTheme } from "./hooks/useTheme";
import { usePreference } from "./hooks/usePreference";
import { api } from "./api";

function beep() {
  try {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.value = 880;
    gain.gain.setValueAtTime(0.0001, ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.12, ctx.currentTime + 0.01);
    gain.gain.exponentialRampToValueAtTime(0.0001, ctx.currentTime + 0.25);
    osc.connect(gain).connect(ctx.destination);
    osc.start();
    osc.stop(ctx.currentTime + 0.26);
  } catch {
    /* audio not available - ignore */
  }
}

function computeHealthScore(snapshot, alerts) {
  if (!snapshot) return null;
  const recentAlerts = alerts?.filter((a) => Date.now() - a.t < 60000).length || 0;
  const score = 100 - snapshot.cpuPercent * 0.45 - snapshot.memory.usedPercent * 0.35 - recentAlerts * 6;
  return Math.max(0, Math.min(100, score));
}

export default function App() {
  const [tab, setTab] = useState("overview");
  const [thresholds, setThresholds] = useState({ cpuPercent: 80, memPercent: 85 });
  const [theme, setTheme] = useTheme();
  const [density, setDensity] = usePreference("km_density", "comfortable", { attribute: "data-density" });
  const [refreshMs, setRefreshMs] = usePreference("km_refresh_ms", "1000");
  const [soundEnabled, setSoundEnabled] = usePreference("km_sound", "false");

  const { data: system } = usePolling(api.system, 5000);
  const { data: snapshot, connected } = usePolling(api.snapshot, Number(refreshMs), [refreshMs]);
  const { data: history } = usePolling(api.history, Math.max(1000, Number(refreshMs) * 2), [refreshMs]);
  const { data: alerts } = usePolling(api.alerts, Math.max(1000, Number(refreshMs) * 2), [refreshMs]);

  useEffect(() => {
    api.getThresholds().then(setThresholds).catch(() => {});
  }, []);

  const prevAlertCount = useRef(0);
  useEffect(() => {
    if (!alerts) return;
    if (alerts.length > prevAlertCount.current && soundEnabled === "true") beep();
    prevAlertCount.current = alerts.length;
  }, [alerts, soundEnabled]);

  const healthScore = computeHealthScore(snapshot, alerts);

  return (
    <div style={{ display: "flex", height: "100vh", fontFamily: "var(--font-sans)" }}>
      <Sidebar active={tab} onChange={setTab} alertCount={alerts?.length || 0} />
      <div style={{ flex: 1, display: "flex", flexDirection: "column", minWidth: 0 }}>
        <Header system={system} connected={connected} healthScore={healthScore} theme={theme} onThemeChange={setTheme} />
        <main style={{ flex: 1, overflow: "auto", padding: "var(--content-pad)" }}>
          {!connected && (
            <div
              style={{
                marginBottom: 16,
                padding: "10px 14px",
                background: "var(--critical-dim)",
                color: "var(--critical)",
                border: "1px solid var(--critical)",
                borderRadius: "var(--radius)",
                fontSize: 12.5,
              }}
            >
              Can't reach the Kernel Monitor backend at {api.baseUrl}. Start it with <code className="mono">npm start</code> in
              the <code className="mono">server</code> folder.
            </div>
          )}
          {tab === "overview" && (
            <Overview snapshot={snapshot} history={history || []} thresholds={thresholds} healthScore={healthScore} />
          )}
          {tab === "processes" && <Processes snapshot={snapshot} />}
          {tab === "alerts" && <Alerts alerts={alerts} />}
          {tab === "history" && <History history={history || []} thresholds={thresholds} />}
          {tab === "settings" && (
            <Settings
              thresholds={thresholds}
              onSaved={setThresholds}
              theme={theme}
              onThemeChange={setTheme}
              density={density}
              onDensityChange={setDensity}
              refreshMs={refreshMs}
              onRefreshChange={setRefreshMs}
              soundEnabled={soundEnabled}
              onSoundChange={setSoundEnabled}
            />
          )}
        </main>
      </div>
    </div>
  );
}
