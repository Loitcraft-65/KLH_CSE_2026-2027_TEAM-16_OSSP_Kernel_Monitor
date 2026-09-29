import { useEffect, useState } from "react";

// Generic localStorage-backed preference, optionally mirrored onto
// document.documentElement as a data-attribute (for CSS to react to).
export function usePreference(key, defaultValue, { attribute } = {}) {
  const [value, setValue] = useState(() => {
    const stored = localStorage.getItem(key);
    return stored ?? defaultValue;
  });

  useEffect(() => {
    localStorage.setItem(key, value);
    if (attribute) document.documentElement.setAttribute(attribute, value);
  }, [key, value, attribute]);

  return [value, setValue];
}
