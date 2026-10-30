/**
 * ============================================================================
 * app.js - Main Frontend Application Logic & Kernel Event Orchestration
 * Lead: Rithwik Rapaka (Roll: 2520030427)
 * Design: High-Responsiveness Telemetry, Subsystem Health Analysis & Dynamic UI
 * ============================================================================
 */

document.addEventListener('DOMContentLoaded', async () => {
    // 1. Initialize Telemetry Charts
    window.telemetryCharts = new TelemetryCharts();

    // 2. DOM Elements Cache
    const elKernel = document.getElementById('meta-kernel');
    const elUptime = document.getElementById('meta-uptime');
    const elConnStatus = document.getElementById('conn-status');
    const elConnText = document.getElementById('conn-text');

    // Controls
    const btnToggleStream = document.getElementById('btn-toggle-stream');
    const iconStream = document.getElementById('icon-stream');
    const textStream = document.getElementById('text-stream');

    // Health Strip
    const elHealthScore = document.getElementById('health-score');
    const elHealthDesc = document.getElementById('health-overall-desc');
    const elHealthSched = document.getElementById('health-sched');
    const elHealthMem = document.getElementById('health-mem');
    const elHealthVm = document.getElementById('health-vm');
    const elHealthVfs = document.getElementById('health-vfs');
    const elHealthIrq = document.getElementById('health-irq');

    // Stat Card 1: Ring-0 Kernel CPU
    const elKernelCpu = document.getElementById('val-kernel-cpu');
    const elUserCpu = document.getElementById('val-user-cpu');
    const elIowait = document.getElementById('val-iowait');
    const elBarKernelCpu = document.getElementById('bar-kernel-cpu');
    const badgeStatusCpu = document.getElementById('badge-status-cpu');

    // Stat Card 2: Kernel Context Switches
    const elCtxtRate = document.getElementById('val-ctxt-rate');
    const elRunnable = document.getElementById('val-runnable');
    const elBlocked = document.getElementById('val-blocked');
    const elBarCtxt = document.getElementById('bar-ctxt');
    const badgeStatusCtxt = document.getElementById('badge-status-ctxt');

    // Stat Card 3: Kernel SLAB Cache
    const elSlabMb = document.getElementById('val-slab-mb');
    const elSlabRec = document.getElementById('val-slab-rec');
    const elSlabRatio = document.getElementById('val-slab-ratio');
    const elBarSlab = document.getElementById('bar-slab');
    const badgeStatusSlab = document.getElementById('badge-status-slab');

    // Stat Card 4: Interrupts & IPIs
    const elResIpis = document.getElementById('val-res-ipis');
    const elCalIpis = document.getElementById('val-cal-ipis');
    const elTlb = document.getElementById('val-tlb');
    const elBarIntr = document.getElementById('bar-intr');
    const badgeStatusIntr = document.getElementById('badge-status-intr');

    // Stat Card 5: VFS Caches
    const elDentryCount = document.getElementById('val-dentry-count');
    const elInodeCount = document.getElementById('val-inode-count');
    const elFileAlloc = document.getElementById('val-file-alloc');
    const elBarVfs = document.getElementById('bar-vfs');
    const badgeStatusVfs = document.getElementById('badge-status-vfs');

    // Tags
    const elTagSchedLoad = document.getElementById('tag-sched-load');
    const elTagForks = document.getElementById('tag-forks');
    const elTagSlabPct = document.getElementById('tag-slab-pct');
    const elTagPgmajfault = document.getElementById('tag-pgmajfault');

    // Tunables
    const elSysctlVfs = document.getElementById('val-sysctl-vfs');
    const elSysctlSwap = document.getElementById('val-sysctl-swap');
    const elSysctlDirty = document.getElementById('val-sysctl-dirty');
    const elSysctlPidmax = document.getElementById('val-sysctl-pidmax');

    // SoftIRQs
    const elSoftirqSched = document.getElementById('val-softirq-sched');
    const elSoftirqRcu = document.getElementById('val-softirq-rcu');
    const elSoftirqTimer = document.getElementById('val-softirq-timer');
    const elSoftirqNet = document.getElementById('val-softirq-net');

    // Alerts
    const elAlertsBanner = document.getElementById('alerts-banner');
    const elAlertTitle = document.getElementById('alert-summary-title');
    const elAlertBadge = document.getElementById('alert-counter-badge');
    const elAlertDesc = document.getElementById('alert-summary-desc');
    const btnAlertReview = document.getElementById('btn-alert-review');

    // Kernel Threads
    const elKthreadsTbody = document.getElementById('kthreads-tbody');
    const elKthreadSearch = document.getElementById('kthread-search');
    const btnClearSearch = document.getElementById('btn-clear-search');
    const elKthreadCount = document.getElementById('kthread-count-label');
    const chipFilters = document.querySelectorAll('#kthread-chip-filters .chip');

    // Modals & Toasts
    const archModal = document.getElementById('arch-modal');
    const btnOpenArch = document.getElementById('btn-open-arch');
    const btnCloseArch = document.getElementById('btn-close-arch');
    const btnCloseArchAction = document.getElementById('btn-close-arch-action');
    const toastContainer = document.getElementById('toast-container');

    // Chart Time Window
    const btnTime30s = document.getElementById('btn-time-30s');
    const btnTime60s = document.getElementById('btn-time-60s');

    let currentKthreads = [];
    let currentFilterChip = '';
    let currentThresholds = {};
    let isStreamPaused = false;
    let rawLogLines = [];
    let activeLogFilter = 'all';

    // Toast Notification Utility
    function showToast(message) {
        if (!toastContainer) return;
        const toast = document.createElement('div');
        toast.className = 'toast';
        toast.textContent = message;
        toastContainer.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateY(10px)';
            toast.style.transition = 'all 0.3s ease';
            setTimeout(() => toast.remove(), 300);
        }, 3000);
    }

    // Architecture Guide Modal
    function openModal() {
        archModal.style.display = 'flex';
    }
    function closeModal() {
        archModal.style.display = 'none';
    }
    if (btnOpenArch) btnOpenArch.addEventListener('click', openModal);
    if (btnCloseArch) btnCloseArch.addEventListener('click', closeModal);
    if (btnCloseArchAction) btnCloseArchAction.addEventListener('click', closeModal);
    archModal.addEventListener('click', (e) => {
        if (e.target === archModal) closeModal();
    });

    // Stream Pause / Resume Toggle
    if (btnToggleStream) {
        btnToggleStream.addEventListener('click', () => {
            isStreamPaused = !isStreamPaused;
            window.telemetryCharts.isPaused = isStreamPaused;
            if (isStreamPaused) {
                iconStream.textContent = '▶️';
                textStream.textContent = 'Resume';
                elConnText.textContent = 'PAUSED';
                elConnStatus.className = 'connection-status disconnected';
                showToast('Telemetry stream paused.');
            } else {
                iconStream.textContent = '⏸️';
                textStream.textContent = 'Pause';
                elConnText.textContent = 'STREAMING (1Hz)';
                elConnStatus.className = 'connection-status';
                showToast('Telemetry stream resumed.');
            }
        });
    }

    // Chart Time Window Buttons
    if (btnTime30s && btnTime60s) {
        btnTime30s.addEventListener('click', () => {
            btnTime30s.classList.add('active');
            btnTime60s.classList.remove('active');
            window.telemetryCharts.setTimeWindow(30);
            showToast('Chart window set to 30 seconds.');
        });
        btnTime60s.addEventListener('click', () => {
            btnTime60s.classList.add('active');
            btnTime30s.classList.remove('active');
            window.telemetryCharts.setTimeWindow(60);
            showToast('Chart window set to 60 seconds.');
        });
    }

    // Tab Switching
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabPanes = document.querySelectorAll('.tab-pane');

    function switchTab(targetId) {
        tabButtons.forEach(b => b.classList.remove('active'));
        tabPanes.forEach(p => p.classList.remove('active'));

        const targetBtn = Array.from(tabButtons).find(b => b.getAttribute('data-tab') === targetId);
        if (targetBtn) targetBtn.classList.add('active');

        const targetPane = document.getElementById(targetId);
        if (targetPane) targetPane.classList.add('active');

        if (targetId === 'tab-logs') {
            refreshLogs();
        }
    }

    tabButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            switchTab(btn.getAttribute('data-tab'));
        });
    });

    if (btnAlertReview) {
        btnAlertReview.addEventListener('click', () => {
            switchTab('tab-tunables');
            document.getElementById('tab-tunables').scrollIntoView({ behavior: 'smooth' });
        });
    }

    // WebSocket Status
    function handleStatusChange(status) {
        if (isStreamPaused) return;

        if (status === 'connected') {
            elConnStatus.className = 'connection-status';
            elConnText.textContent = 'STREAMING (1Hz)';
        } else if (status === 'connecting') {
            elConnStatus.className = 'connection-status';
            elConnText.textContent = 'CONNECTING...';
        } else {
            elConnStatus.className = 'connection-status disconnected';
            elConnText.textContent = 'DISCONNECTED';
        }
    }

    // Telemetry Update Handler
    function handleTelemetry(snap) {
        if (isStreamPaused) return;

        if (snap.kernel_version) elKernel.textContent = snap.kernel_version;
        if (snap.uptime !== undefined) {
            const hrs = Math.floor(snap.uptime / 3600);
            const mins = Math.floor((snap.uptime % 3600) / 60);
            const secs = snap.uptime % 60;
            elUptime.textContent = `${hrs}h ${mins}m ${secs}s`;
        }

        // 1. CPU Scheduler
        const sched = snap.sched || {};
        const sysCpu = sched.system_pct || 0;
        elKernelCpu.textContent = sysCpu.toFixed(1);
        elUserCpu.textContent = `${(sched.user_pct || 0).toFixed(1)}%`;
        elIowait.textContent = `${(sched.iowait_pct || 0).toFixed(1)}%`;
        elBarKernelCpu.style.width = `${Math.min(100, sysCpu * 2.8)}%`;

        if (sysCpu > 35) {
            badgeStatusCpu.className = 'status-pill pill-warn';
            badgeStatusCpu.textContent = 'Elevated';
        } else {
            badgeStatusCpu.className = 'status-pill pill-ok';
            badgeStatusCpu.textContent = 'Normal';
        }

        const ctxtRate = sched.ctxt_rate || 0;
        elCtxtRate.textContent = Math.round(ctxtRate).toLocaleString();
        elRunnable.textContent = `${sched.runnable || 0} tasks`;
        elBlocked.textContent = `${sched.procs_blocked || 0}`;
        elBarCtxt.style.width = `${Math.min(100, (ctxtRate / 22000) * 100)}%`;

        if (ctxtRate > 18000) {
            badgeStatusCtxt.className = 'status-pill pill-warn';
            badgeStatusCtxt.textContent = 'High Churn';
        } else {
            badgeStatusCtxt.className = 'status-pill pill-ok';
            badgeStatusCtxt.textContent = 'Optimal';
        }

        elTagSchedLoad.textContent = `Load: ${sched.load_1m || 0.00}`;
        elTagForks.textContent = `Forks: ${(sched.processes_forked || 0).toLocaleString()}`;

        // 2. Kernel Memory & SLAB
        const mem = snap.memory || {};
        const slabMb = Math.round((mem.slab_total_kb || 0) / 1024);
        const slabRecMb = Math.round((mem.slab_reclaim_kb || 0) / 1024);
        const slabRatio = mem.slab_ratio_pct || 0;
        elSlabMb.textContent = slabMb.toLocaleString();
        elSlabRec.textContent = `${slabRecMb.toLocaleString()} MB`;
        elSlabRatio.textContent = `${slabRatio.toFixed(1)}%`;
        elBarSlab.style.width = `${Math.min(100, slabRatio * 4.5)}%`;

        if (slabRatio > 25) {
            badgeStatusSlab.className = 'status-pill pill-warn';
            badgeStatusSlab.textContent = 'Heavy';
        } else {
            badgeStatusSlab.className = 'status-pill pill-ok';
            badgeStatusSlab.textContent = 'Normal';
        }

        elTagSlabPct.textContent = `Slab: ${slabRatio.toFixed(1)}%`;
        elTagPgmajfault.textContent = `MajFaults: ${Math.round(mem.pgmajfault_rate || 0)}/s`;

        // 3. Interrupts & IPIs
        const intr = snap.intr || {};
        elResIpis.textContent = (intr.res_ipis || 0).toLocaleString();
        elCalIpis.textContent = (intr.cal_ipis || 0).toLocaleString();
        elTlb.textContent = (intr.tlb_shootdowns || 0).toLocaleString();
        elBarIntr.style.width = `${Math.min(100, Math.max(15, (intr.res_ipis % 1000) / 10))}%`;

        // 4. VFS Caches
        const vfs = snap.vfs || {};
        elDentryCount.textContent = (vfs.dentry_count || 0).toLocaleString();
        elInodeCount.textContent = (vfs.inode_count || 0).toLocaleString();
        elFileAlloc.textContent = `${(vfs.allocated_files || 0).toLocaleString()} files`;
        elBarVfs.style.width = `${Math.min(100, Math.max(10, (vfs.alloc_pct || 0.001) * 20000))}%`;

        // 5. Tunables
        const tunables = snap.tunables || {};
        if (tunables.vfs_cache_pressure !== undefined) elSysctlVfs.textContent = tunables.vfs_cache_pressure;
        if (tunables.swappiness !== undefined) elSysctlSwap.textContent = tunables.swappiness;
        if (tunables.dirty_ratio !== undefined) elSysctlDirty.textContent = `${tunables.dirty_ratio}%`;
        if (tunables.pid_max !== undefined) elSysctlPidmax.textContent = tunables.pid_max.toLocaleString();

        // 6. SoftIRQs
        if (intr.softirq_sched !== undefined) elSoftirqSched.textContent = intr.softirq_sched.toLocaleString();
        if (intr.softirq_rcu !== undefined) elSoftirqRcu.textContent = intr.softirq_rcu.toLocaleString();
        if (intr.softirq_timer !== undefined) elSoftirqTimer.textContent = intr.softirq_timer.toLocaleString();
        if (intr.softirq_net_rx !== undefined) elSoftirqNet.textContent = intr.softirq_net_rx.toLocaleString();

        // Update Charts
        window.telemetryCharts.updateTelemetry(snap);

        // Update Kernel Threads
        currentKthreads = snap.kthreads || [];
        renderKthreadsTable();

        // Check Alerts
        checkAlertStatus();
    }

    // Render Kernel Threads Table
    function renderKthreadsTable() {
        const query = (elKthreadSearch.value || '').toLowerCase().trim();
        const filtered = currentKthreads.filter(t => {
            const matchesChip = !currentFilterChip || (t.name || '').toLowerCase().includes(currentFilterChip.toLowerCase());
            if (!matchesChip) return false;
            if (!query) return true;
            return (t.name || '').toLowerCase().includes(query) || String(t.pid).includes(query);
        });

        elKthreadCount.textContent = `Showing ${filtered.length} of ${currentKthreads.length} ring-0 threads`;

        if (filtered.length === 0) {
            elKthreadsTbody.innerHTML = `<tr><td colspan="7" class="loading-cell">No kernel threads matching filter "${currentFilterChip || query}"</td></tr>`;
            return;
        }

        let html = '';
        filtered.forEach(t => {
            const stateLetter = (t.state || 'S').toUpperCase();
            const stateClass = stateLetter === 'R' ? 'state-r' : 'state-s';
            const stateName = stateLetter === 'R' ? 'Running' : (stateLetter === 'D' ? 'Disk Wait' : 'Sleeping');

            html += `
                <tr>
                    <td class="mono"><strong>${t.pid}</strong></td>
                    <td class="mono text-muted">${t.ppid}</td>
                    <td><span class="kthread-badge">RING-0 KTHREAD</span></td>
                    <td class="mono"><strong>${t.name}</strong></td>
                    <td class="mono">${((t.vol_ctxt ?? t.voluntary_ctxt) || 0).toLocaleString()}</td>
                    <td class="mono">${((t.invol_ctxt ?? t.nonvoluntary_ctxt) || 0).toLocaleString()}</td>
                    <td><span class="kthread-state-badge ${stateClass}">${stateLetter} (${stateName})</span></td>
                </tr>
            `;
        });
        elKthreadsTbody.innerHTML = html;
    }

    // Chip Filter Event Handling
    chipFilters.forEach(chip => {
        chip.addEventListener('click', () => {
            chipFilters.forEach(c => c.classList.remove('active'));
            chip.classList.add('active');
            currentFilterChip = chip.getAttribute('data-filter') || '';
            renderKthreadsTable();
        });
    });

    // Search Input Handling
    elKthreadSearch.addEventListener('input', () => {
        btnClearSearch.style.display = elKthreadSearch.value ? 'block' : 'none';
        renderKthreadsTable();
    });

    btnClearSearch.addEventListener('click', () => {
        elKthreadSearch.value = '';
        btnClearSearch.style.display = 'none';
        renderKthreadsTable();
    });

    // Alerts Check & Health Score Calculation
    async function checkAlertStatus() {
        try {
            const res = await fetch('/api/alerts');
            const data = await res.json();
            const active = data.active_alerts || [];

            if (active.length === 0) {
                elAlertsBanner.className = 'alerts-banner';
                elAlertTitle.textContent = 'KERNEL SUBSYSTEMS STATUS OPTIMAL';
                elAlertBadge.textContent = '0 Alerts Active';
                elAlertDesc.textContent = 'Ring-0 scheduler dispatch, SLAB caches, MMU page faulting, and VFS file tables are within operating limits.';
                elHealthScore.textContent = '98%';
                elHealthDesc.textContent = 'All Ring-0 subsystems operating within normal parameters.';
                updateSubsystemStatus(elHealthSched, 'ok', 'Optimal');
                updateSubsystemStatus(elHealthMem, 'ok', 'Normal');
                updateSubsystemStatus(elHealthVm, 'ok', 'Stable');
                updateSubsystemStatus(elHealthVfs, 'ok', 'Nominal');
                updateSubsystemStatus(elHealthIrq, 'ok', 'Active');
            } else {
                const hasCrit = active.some(a => a.severity === 'critical');
                elAlertsBanner.className = hasCrit ? 'alerts-banner has-critical' : 'alerts-banner has-warning';
                elAlertTitle.textContent = `${active.length} KERNEL SUBSYSTEM ALERT(S) DETECTED`;
                elAlertBadge.textContent = `${active.length} Active`;
                elAlertDesc.textContent = active.map(a => `[${a.subsystem}] ${a.message}`).join(' | ');

                const penalty = Math.min(40, active.length * 15);
                elHealthScore.textContent = `${98 - penalty}%`;
                elHealthDesc.textContent = `Warning: ${active.length} threshold limit(s) triggered across subsystems.`;

                active.forEach(a => {
                    const subLower = (a.subsystem || '').toLowerCase();
                    const state = a.severity === 'critical' ? 'crit' : 'warn';
                    if (subLower.includes('cpu') || subLower.includes('scheduler')) updateSubsystemStatus(elHealthSched, state, 'Attention');
                    if (subLower.includes('memory') || subLower.includes('slab')) updateSubsystemStatus(elHealthMem, state, 'Elevated');
                    if (subLower.includes('page') || subLower.includes('vm')) updateSubsystemStatus(elHealthVm, state, 'Faulting');
                    if (subLower.includes('vfs') || subLower.includes('file')) updateSubsystemStatus(elHealthVfs, state, 'High Alloc');
                });
            }
        } catch (e) {}
    }

    function updateSubsystemStatus(elem, state, text) {
        if (!elem) return;
        const dot = elem.querySelector('.sub-dot');
        const val = elem.querySelector('.sub-val');
        if (dot) dot.className = `sub-dot ${state}`;
        if (val) val.textContent = text;
    }

    // Thresholds & Presets
    async function loadThresholds() {
        currentThresholds = await window.api.getThresholds();
        if (!currentThresholds) return;

        bindSlider('th-kernel-cpu', 'val-th-kernel-cpu', currentThresholds.max_kernel_cpu_pct, '%');
        bindSlider('th-ctxt', 'val-th-ctxt', currentThresholds.max_ctxt_rate_sec, '/s');
        bindSlider('th-slab', 'val-th-slab', currentThresholds.max_slab_pct, '%');
        bindSlider('th-pgfault', 'val-th-pgfault', currentThresholds.max_pgfault_rate_sec, '/s');
        bindSlider('th-pgmajfault', 'val-th-pgmajfault', currentThresholds.max_pgmajfault_rate_sec, '/s');
        bindSlider('th-vfs', 'val-th-vfs', currentThresholds.max_file_descriptor_pct, '%');
    }

    function bindSlider(inputId, valId, initialVal, suffix) {
        const input = document.getElementById(inputId);
        const label = document.getElementById(valId);
        if (!input || !label) return;

        input.value = initialVal;
        label.textContent = `${initialVal}${suffix}`;

        input.oninput = () => {
            label.textContent = `${input.value}${suffix}`;
        };
    }

    function setSliderValues(kernelCpu, ctxt, slab, pgfault, pgmajfault, vfs) {
        setVal('th-kernel-cpu', 'val-th-kernel-cpu', kernelCpu, '%');
        setVal('th-ctxt', 'val-th-ctxt', ctxt, '/s');
        setVal('th-slab', 'val-th-slab', slab, '%');
        setVal('th-pgfault', 'val-th-pgfault', pgfault, '/s');
        setVal('th-pgmajfault', 'val-th-pgmajfault', pgmajfault, '/s');
        setVal('th-vfs', 'val-th-vfs', vfs, '%');
    }

    function setVal(id, labelId, val, suffix) {
        const input = document.getElementById(id);
        const label = document.getElementById(labelId);
        if (input) input.value = val;
        if (label) label.textContent = `${val}${suffix}`;
    }

    // Threshold Presets
    document.getElementById('preset-default').addEventListener('click', () => {
        setSliderValues(30, 15000, 20, 8000, 50, 75);
        showToast('Applied Standard Profile presets.');
    });

    document.getElementById('preset-server').addEventListener('click', () => {
        setSliderValues(45, 30000, 35, 18000, 100, 85);
        showToast('Applied High-Load Server presets.');
    });

    document.getElementById('preset-strict').addEventListener('click', () => {
        setSliderValues(15, 8000, 12, 3500, 15, 50);
        showToast('Applied Strict Auditing presets.');
    });

    document.getElementById('btn-save-thresholds').addEventListener('click', async (e) => {
        e.preventDefault();
        const payload = {
            max_kernel_cpu_pct: parseFloat(document.getElementById('th-kernel-cpu').value),
            max_ctxt_rate_sec: parseFloat(document.getElementById('th-ctxt').value),
            max_slab_pct: parseFloat(document.getElementById('th-slab').value),
            max_pgfault_rate_sec: parseFloat(document.getElementById('th-pgfault').value),
            max_pgmajfault_rate_sec: parseFloat(document.getElementById('th-pgmajfault').value),
            max_file_descriptor_pct: parseFloat(document.getElementById('th-vfs').value),
            max_dentry_unused: 50000
        };

        const res = await window.api.updateThresholds(payload);
        if (res && res.message) {
            showToast('Kernel threshold rules saved & active.');
        }
    });

    // Logs Viewer & Filter
    async function refreshLogs() {
        const viewer = document.getElementById('log-viewer');
        const res = await window.api.getLogs(80);
        if (res && res.logs) {
            rawLogLines = res.logs;
            renderLogs();
        }
    }

    function renderLogs() {
        const viewer = document.getElementById('log-viewer');
        let filtered = rawLogLines;
        if (activeLogFilter === 'warn') {
            filtered = rawLogLines.filter(l => l.includes('[WARNING]'));
        } else if (activeLogFilter === 'crit') {
            filtered = rawLogLines.filter(l => l.includes('[CRITICAL]'));
        }

        if (filtered.length === 0) {
            viewer.textContent = `No logs recorded matching filter "${activeLogFilter}".`;
            return;
        }

        viewer.textContent = filtered.join('');
        viewer.scrollTop = viewer.scrollHeight;
    }

    document.getElementById('btn-refresh-logs').addEventListener('click', () => {
        refreshLogs();
        showToast('Logs refreshed.');
    });

    document.getElementById('btn-copy-logs').addEventListener('click', () => {
        const viewer = document.getElementById('log-viewer');
        navigator.clipboard.writeText(viewer.textContent).then(() => {
            showToast('Logs copied to clipboard.');
        });
    });

    // Log Filter Buttons
    const btnLogAll = document.getElementById('btn-log-all');
    const btnLogWarn = document.getElementById('btn-log-warn');
    const btnLogCrit = document.getElementById('btn-log-crit');

    [btnLogAll, btnLogWarn, btnLogCrit].forEach(btn => {
        if (!btn) return;
        btn.addEventListener('click', () => {
            [btnLogAll, btnLogWarn, btnLogCrit].forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            if (btn === btnLogAll) activeLogFilter = 'all';
            else if (btn === btnLogWarn) activeLogFilter = 'warn';
            else if (btn === btnLogCrit) activeLogFilter = 'crit';
            renderLogs();
        });
    });

    // Initial Load & Stream Connect
    await loadThresholds();
    window.api.connectWebSocket(handleTelemetry, handleStatusChange);
});
