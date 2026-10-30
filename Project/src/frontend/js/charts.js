/**
 * ============================================================================
 * charts.js - Real-time Kernel Subsystem Telemetry Charts (Chart.js)
 * Lead: Rithwik Rapaka (Roll: 2520030427)
 * Design: High-Precision Smooth Waveforms with Vertical Gradient Area Fills
 * ============================================================================
 */

class TelemetryCharts {
    constructor() {
        this.maxDataPoints = 30;
        this.isPaused = false;
        this.chartCpu = null;
        this.chartCtxt = null;
        this.chartMem = null;
        this.chartVm = null;
        this.initCharts();
    }

    createGradient(ctx, colorTop, colorBottom = 'rgba(0,0,0,0)') {
        const gradient = ctx.createLinearGradient(0, 0, 0, 240);
        gradient.addColorStop(0, colorTop);
        gradient.addColorStop(1, colorBottom);
        return gradient;
    }

    createDefaultOptions(yUnit = '') {
        return {
            responsive: true,
            maintainAspectRatio: false,
            animation: {
                duration: 400,
                easing: 'easeOutCubic'
            },
            interaction: {
                intersect: false,
                mode: 'index'
            },
            plugins: {
                legend: {
                    position: 'top',
                    align: 'end',
                    labels: {
                        color: '#94a3b8',
                        font: { family: "'Outfit', sans-serif", size: 11, weight: '500' },
                        boxWidth: 10,
                        boxHeight: 10,
                        usePointStyle: true,
                        pointStyle: 'circle',
                        padding: 12
                    }
                },
                tooltip: {
                    backgroundColor: 'rgba(10, 16, 30, 0.94)',
                    titleColor: '#00f2fe',
                    bodyColor: '#f8fafc',
                    borderColor: 'rgba(255, 255, 255, 0.12)',
                    borderWidth: 1,
                    cornerRadius: 10,
                    padding: 12,
                    boxPadding: 6,
                    usePointStyle: true,
                    titleFont: { family: "'Outfit', sans-serif", size: 12, weight: '700' },
                    bodyFont: { family: "'JetBrains Mono', monospace", size: 11 },
                    callbacks: {
                        label: function (context) {
                            let label = context.dataset.label || '';
                            if (label) label += ': ';
                            if (context.parsed.y !== null) {
                                label += context.parsed.y.toLocaleString() + yUnit;
                            }
                            return label;
                        }
                    }
                }
            },
            scales: {
                x: {
                    grid: {
                        color: 'rgba(255, 255, 255, 0.04)',
                        tickLength: 4
                    },
                    ticks: {
                        color: '#64748b',
                        font: { family: "'JetBrains Mono', monospace", size: 10 },
                        maxRotation: 0,
                        maxTicksLimit: 7
                    }
                },
                y: {
                    grid: {
                        color: 'rgba(255, 255, 255, 0.04)',
                        tickLength: 4
                    },
                    ticks: {
                        color: '#64748b',
                        font: { family: "'JetBrains Mono', monospace", size: 10 },
                        callback: (v) => `${v}${yUnit}`
                    },
                    suggestedMin: 0
                }
            }
        };
    }

    initCharts() {
        const labels = Array(this.maxDataPoints).fill('');

        // 1. CPU Execution Rings: Ring-0 Kernel Mode vs User Space vs I/O Wait
        const ctxCpuElem = document.getElementById('chart-cpu');
        if (ctxCpuElem) {
            const ctxCpu = ctxCpuElem.getContext('2d');
            const gradCrimson = this.createGradient(ctxCpu, 'rgba(239, 68, 68, 0.28)');
            const gradCyan = this.createGradient(ctxCpu, 'rgba(0, 242, 254, 0.15)');

            this.chartCpu = new Chart(ctxCpu, {
                type: 'line',
                data: {
                    labels: [...labels],
                    datasets: [
                        {
                            label: 'Ring-0 Kernel Space (System %)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#ef4444',
                            backgroundColor: gradCrimson,
                            fill: true,
                            tension: 0.42,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointHoverBackgroundColor: '#ef4444'
                        },
                        {
                            label: 'Ring-3 User Space (%)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#00f2fe',
                            backgroundColor: gradCyan,
                            fill: true,
                            tension: 0.42,
                            borderWidth: 1.8,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointHoverBackgroundColor: '#00f2fe'
                        },
                        {
                            label: 'I/O Wait Queue (%)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#f59e0b',
                            borderDash: [4, 4],
                            fill: false,
                            tension: 0.4,
                            borderWidth: 1.5,
                            pointRadius: 0,
                            pointHoverRadius: 3
                        }
                    ]
                },
                options: {
                    ...this.createDefaultOptions('%'),
                    scales: {
                        ...this.createDefaultOptions('%').scales,
                        y: { ...this.createDefaultOptions('%').scales.y, max: 100 }
                    }
                }
            });
        }

        // 2. Scheduler Dispatch & Rescheduling IPIs
        const ctxCtxtElem = document.getElementById('chart-ctxt');
        if (ctxCtxtElem) {
            const ctxCtxt = ctxCtxtElem.getContext('2d');
            const gradPurple = this.createGradient(ctxCtxt, 'rgba(168, 85, 247, 0.24)');

            this.chartCtxt = new Chart(ctxCtxt, {
                type: 'line',
                data: {
                    labels: [...labels],
                    datasets: [
                        {
                            label: 'Context Switches / sec',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#a855f7',
                            backgroundColor: gradPurple,
                            fill: true,
                            tension: 0.38,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointHoverBackgroundColor: '#a855f7'
                        },
                        {
                            label: 'Rescheduling IPIs (RES / sec)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#38bdf8',
                            fill: false,
                            tension: 0.38,
                            borderWidth: 1.6,
                            pointRadius: 0,
                            pointHoverRadius: 3
                        }
                    ]
                },
                options: this.createDefaultOptions()
            });
        }

        // 3. Kernel Memory Footprint: SLAB, PageTables, Kernel Stacks
        const ctxMemElem = document.getElementById('chart-mem');
        if (ctxMemElem) {
            const ctxMem = ctxMemElem.getContext('2d');
            const gradEmerald = this.createGradient(ctxMem, 'rgba(16, 185, 129, 0.22)');

            this.chartMem = new Chart(ctxMem, {
                type: 'line',
                data: {
                    labels: [...labels],
                    datasets: [
                        {
                            label: 'Kernel SLAB (MB)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#10b981',
                            backgroundColor: gradEmerald,
                            fill: true,
                            tension: 0.38,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointHoverBackgroundColor: '#10b981'
                        },
                        {
                            label: 'MMU PageTables (MB)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#38bdf8',
                            fill: false,
                            tension: 0.38,
                            borderWidth: 1.6,
                            pointRadius: 0,
                            pointHoverRadius: 3
                        },
                        {
                            label: 'Kernel Thread Stacks (MB)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#fbbf24',
                            borderDash: [3, 3],
                            fill: false,
                            tension: 0.38,
                            borderWidth: 1.5,
                            pointRadius: 0,
                            pointHoverRadius: 3
                        }
                    ]
                },
                options: this.createDefaultOptions(' MB')
            });
        }

        // 4. Virtual Memory MMU Faults: Minor vs Major
        const ctxVmElem = document.getElementById('chart-vm');
        if (ctxVmElem) {
            const ctxVm = ctxVmElem.getContext('2d');
            const gradAmber = this.createGradient(ctxVm, 'rgba(245, 158, 11, 0.18)');

            this.chartVm = new Chart(ctxVm, {
                type: 'line',
                data: {
                    labels: [...labels],
                    datasets: [
                        {
                            label: 'Minor Page Faults (Soft / sec)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#f59e0b',
                            backgroundColor: gradAmber,
                            fill: true,
                            tension: 0.38,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointHoverBackgroundColor: '#f59e0b'
                        },
                        {
                            label: 'Major Faults (Disk page-ins / sec)',
                            data: Array(this.maxDataPoints).fill(0),
                            borderColor: '#f43f5e',
                            borderWidth: 2,
                            fill: false,
                            tension: 0.2,
                            pointRadius: 2.5,
                            pointBackgroundColor: '#f43f5e',
                            pointHoverRadius: 5
                        }
                    ]
                },
                options: this.createDefaultOptions('/s')
            });
        }
    }

    setTimeWindow(points) {
        this.maxDataPoints = points;
        const allCharts = [this.chartCpu, this.chartCtxt, this.chartMem, this.chartVm];
        allCharts.forEach(c => {
            if (!c) return;
            while (c.data.labels.length > points) {
                c.data.labels.shift();
                c.data.datasets.forEach(ds => ds.data.shift());
            }
            c.update();
        });
    }

    pushData(chart, values, label = '') {
        if (!chart || this.isPaused) return;

        chart.data.labels.push(label);
        if (chart.data.labels.length > this.maxDataPoints) {
            chart.data.labels.shift();
        }

        values.forEach((val, idx) => {
            if (chart.data.datasets[idx]) {
                chart.data.datasets[idx].data.push(val);
                if (chart.data.datasets[idx].data.length > this.maxDataPoints) {
                    chart.data.datasets[idx].data.shift();
                }
            }
        });
        chart.update('none');
    }

    updateTelemetry(snap) {
        if (this.isPaused) return;

        const timeLabel = new Date(snap.timestamp * 1000).toLocaleTimeString([], {
            hour12: false,
            minute: '2-digit',
            second: '2-digit'
        });

        // 1. CPU Modes
        const sched = snap.sched || {};
        this.pushData(
            this.chartCpu,
            [sched.system_pct || 0, sched.user_pct || 0, sched.iowait_pct || 0],
            timeLabel
        );

        // 2. Context Switches & RES IPIs
        const intr = snap.intr || {};
        this.pushData(
            this.chartCtxt,
            [sched.ctxt_rate || 0, intr.res_ipis_rate || 0],
            timeLabel
        );

        // 3. Kernel Memory
        const mem = snap.memory || {};
        const slabMb = roundNum((mem.slab_total_kb || 0) / 1024, 1);
        const ptMb = roundNum((mem.page_tables_kb || 0) / 1024, 1);
        const stackMb = roundNum((mem.kernel_stack_kb || 0) / 1024, 1);
        this.pushData(this.chartMem, [slabMb, ptMb, stackMb], timeLabel);

        // 4. Page Faults
        this.pushData(this.chartVm, [mem.pgfault_rate || 0, mem.pgmajfault_rate || 0], timeLabel);
    }
}

function roundNum(num, dec = 1) {
    return Math.round(num * Math.pow(10, dec)) / Math.pow(10, dec);
}

window.telemetryCharts = null;
