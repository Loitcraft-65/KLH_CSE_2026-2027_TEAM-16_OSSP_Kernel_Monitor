import { useEffect, useState } from "react";

const STORAGE_KEY = "km_theme"; // "light" | "dark" | "system"

function resolveSystemTheme() {
  return window.matchMedia("(prefers-color-scheme: light)").matches ? "light" : "dark";
}

export function useTheme() {
  const [theme, setTheme] = useState(() => localStorage.getItem(STORAGE_KEY) || "dark");

  useEffect(() => {
    const applied = theme === "system" ? resolveSystemTheme() : theme;
    document.documentElement.setAttribute("data-theme", applied);
    localStorage.setItem(STORAGE_KEY, theme);
  }, [theme]);

  useEffect(() => {
    if (theme !== "system") return;
    const mq = window.matchMedia("(prefers-color-scheme: light)");
    const handler = () => document.documentElement.setAttribute("data-theme", resolveSystemTheme());
    mq.addEventListener("change", handler);
    return () => mq.removeEventListener("change", handler);
  }, [theme]);

  return [theme, setTheme];
}
