import { useEffect, useRef, useState } from "react";

// Polls `fn` every `intervalMs`, exposing the latest result, a connection
// flag, and any error. Pauses cleanly on unmount.
export function usePolling(fn, intervalMs = 1000, deps = []) {
  const [data, setData] = useState(null);
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState(null);
  const fnRef = useRef(fn);
  fnRef.current = fn;

  useEffect(() => {
    let alive = true;
    let timer;

    async function tick() {
      try {
        const result = await fnRef.current();
        if (!alive) return;
        setData(result);
        setConnected(true);
        setError(null);
      } catch (err) {
        if (!alive) return;
        setConnected(false);
        setError(err.message || "connection failed");
      } finally {
        if (alive) timer = setTimeout(tick, intervalMs);
      }
    }

    tick();
    return () => {
      alive = false;
      clearTimeout(timer);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, deps);

  return { data, connected, error };
}
