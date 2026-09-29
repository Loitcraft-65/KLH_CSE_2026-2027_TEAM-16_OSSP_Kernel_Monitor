const BASE_URL = import.meta.env.VITE_API_URL || "http://localhost:5000";

async function get(path) {
  const res = await fetch(`${BASE_URL}${path}`);
  if (!res.ok) throw new Error(`${path} -> ${res.status}`);
  return res.json();
}

export const api = {
  system: () => get("/api/system"),
  snapshot: () => get("/api/snapshot"),
  history: () => get("/api/history"),
  alerts: () => get("/api/alerts"),
  getThresholds: () => get("/api/thresholds"),
  setThresholds: (thresholds) =>
    fetch(`${BASE_URL}/api/thresholds`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(thresholds),
    }).then((r) => r.json()),
  exportJsonUrl: () => `${BASE_URL}/api/export.json`,
  exportCsvUrl: () => `${BASE_URL}/api/export.csv`,
  baseUrl: BASE_URL,
};
