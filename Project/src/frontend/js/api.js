/**
 * ============================================================================
 * api.js - REST API Client & Live WebSocket Connection Manager
 * Lead: Saahas Vaddi & Rithwik Rapaka
 * ============================================================================
 */

class KernelMonitorAPI {
    constructor() {
        this.host = window.location.host;
        this.protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        this.wsUrl = `${this.protocol}//${this.host}/ws/live`;
        this.ws = null;
        this.onTelemetryCallback = null;
        this.onStatusChangeCallback = null;
        this.reconnectTimer = null;
    }

    connectWebSocket(onTelemetry, onStatusChange) {
        this.onTelemetryCallback = onTelemetry;
        this.onStatusChangeCallback = onStatusChange;

        if (this.ws) {
            try { this.ws.close(); } catch(e) {}
        }

        if (this.onStatusChangeCallback) this.onStatusChangeCallback('connecting');

        this.ws = new WebSocket(this.wsUrl);

        this.ws.onopen = () => {
            if (this.onStatusChangeCallback) this.onStatusChangeCallback('connected');
            if (this.reconnectTimer) {
                clearTimeout(this.reconnectTimer);
                this.reconnectTimer = null;
            }
        };

        this.ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (this.onTelemetryCallback) {
                    this.onTelemetryCallback(data);
                }
            } catch (err) {
                console.error("[WS Parse Error]", err);
            }
        };

        this.ws.onclose = () => {
            if (this.onStatusChangeCallback) this.onStatusChangeCallback('disconnected');
            this.scheduleReconnect();
        };

        this.ws.onerror = (err) => {
            console.warn("[WS Error]", err);
            if (this.onStatusChangeCallback) this.onStatusChangeCallback('disconnected');
        };
    }

    scheduleReconnect() {
        if (!this.reconnectTimer) {
            this.reconnectTimer = setTimeout(() => {
                this.reconnectTimer = null;
                this.connectWebSocket(this.onTelemetryCallback, this.onStatusChangeCallback);
            }, 2000);
        }
    }

    async getHistory(limit = 60) {
        try {
            const res = await fetch(`/api/metrics/history?limit=${limit}`);
            return await res.json();
        } catch (e) {
            console.error("[API Error] getHistory:", e);
            return [];
        }
    }

    async getThresholds() {
        try {
            const res = await fetch('/api/thresholds');
            return await res.json();
        } catch (e) {
            console.error("[API Error] getThresholds:", e);
            return null;
        }
    }

    async updateThresholds(config) {
        try {
            const res = await fetch('/api/thresholds', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            return await res.json();
        } catch (e) {
            console.error("[API Error] updateThresholds:", e);
            return null;
        }
    }

    async getLogs(lines = 50) {
        try {
            const res = await fetch(`/api/logs?lines=${lines}`);
            return await res.json();
        } catch (e) {
            console.error("[API Error] getLogs:", e);
            return { logs: ["Error fetching logs."] };
        }
    }
}

window.api = new KernelMonitorAPI();
