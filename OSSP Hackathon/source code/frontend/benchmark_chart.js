/**
 * High-DPI Canvas-based Speedup & Benchmark Chart
 * Compares Measured Multi-Thread Speedup vs Amdahl's Law Theoretical Curve
 */
class BenchmarkChart {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    if (!this.canvas) return;
    this.ctx = this.canvas.getContext('2d');
    this.data = null;
    this.initResizeListener();
  }

  initResizeListener() {
    window.addEventListener('resize', () => {
      if (this.data) this.render(this.data);
    });
  }

  render(benchmarkData) {
    this.data = benchmarkData;
    if (!this.canvas || !benchmarkData || !benchmarkData.results) return;

    const rect = this.canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const width = rect.width;
    const height = rect.height;

    this.canvas.width = width * dpr;
    this.canvas.height = height * dpr;
    this.ctx.scale(dpr, dpr);

    const ctx = this.ctx;
    ctx.clearRect(0, 0, width, height);

    const results = benchmarkData.results;
    if (results.length === 0) return;

    const padLeft = 55;
    const padRight = 30;
    const padTop = 35;
    const padBottom = 40;
    const chartW = width - padLeft - padRight;
    const chartH = height - padTop - padBottom;

    // Calculate max speedup
    const maxThreads = Math.max(...results.map(r => r.threads));
    const maxSpeedup = Math.max(
      ...results.map(r => Math.max(r.speedup, r.amdahl_speedup)),
      maxThreads * 0.8,
      2
    );

    // Draw Grid Lines
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.06)';
    ctx.lineWidth = 1;
    const ySteps = 4;
    for (let i = 0; i <= ySteps; i++) {
      const yVal = (maxSpeedup / ySteps) * i;
      const yPos = padTop + chartH - (i / ySteps) * chartH;
      ctx.beginPath();
      ctx.moveTo(padLeft, yPos);
      ctx.lineTo(width - padRight, yPos);
      ctx.stroke();

      // Label
      ctx.fillStyle = '#64748b';
      ctx.font = '11px "JetBrains Mono", monospace';
      ctx.textAlign = 'right';
      ctx.fillText(yVal.toFixed(1) + 'x', padLeft - 10, yPos + 4);
    }

    // X positions helper
    const getX = (idx) => padLeft + (idx / (results.length - 1)) * chartW;
    const getY = (speedup) => padTop + chartH - (speedup / maxSpeedup) * chartH;

    // Draw Amdahl's Law Line (Theoretical)
    ctx.beginPath();
    ctx.setLineDash([5, 5]);
    ctx.strokeStyle = '#a855f7';
    ctx.lineWidth = 2;
    results.forEach((r, idx) => {
      const x = getX(idx);
      const y = getY(r.amdahl_speedup);
      if (idx === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();
    ctx.setLineDash([]);

    // Draw Measured Speedup Area fill
    ctx.beginPath();
    results.forEach((r, idx) => {
      const x = getX(idx);
      const y = getY(r.speedup);
      if (idx === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.lineTo(getX(results.length - 1), padTop + chartH);
    ctx.lineTo(getX(0), padTop + chartH);
    ctx.closePath();
    const grad = ctx.createLinearGradient(0, padTop, 0, padTop + chartH);
    grad.addColorStop(0, 'rgba(0, 242, 254, 0.25)');
    grad.addColorStop(1, 'rgba(0, 242, 254, 0.0)');
    ctx.fillStyle = grad;
    ctx.fill();

    // Draw Measured Speedup Solid Line
    ctx.beginPath();
    ctx.strokeStyle = '#00f2fe';
    ctx.lineWidth = 3;
    results.forEach((r, idx) => {
      const x = getX(idx);
      const y = getY(r.speedup);
      if (idx === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();

    // Draw Data Points & Thread Labels
    results.forEach((r, idx) => {
      const x = getX(idx);
      const y = getY(r.speedup);

      // Glowing dot
      ctx.fillStyle = '#00f2fe';
      ctx.shadowColor = '#00f2fe';
      ctx.shadowBlur = 10;
      ctx.beginPath();
      ctx.arc(x, y, 5, 0, Math.PI * 2);
      ctx.fill();
      ctx.shadowBlur = 0; // reset shadow

      // White inner dot
      ctx.fillStyle = '#ffffff';
      ctx.beginPath();
      ctx.arc(x, y, 2.5, 0, Math.PI * 2);
      ctx.fill();

      // Value label on top of point
      ctx.fillStyle = '#f1f5f9';
      ctx.font = 'bold 11px "JetBrains Mono", monospace';
      ctx.textAlign = 'center';
      ctx.fillText(r.speedup + 'x', x, y - 10);

      // X Axis Thread label below
      ctx.fillStyle = '#94a3b8';
      ctx.font = '11px "Plus Jakarta Sans", sans-serif';
      ctx.fillText(r.threads + 'T', x, padTop + chartH + 20);
    });

    // Legend
    const legendX = padLeft + 10;
    const legendY = 18;

    // Measured
    ctx.strokeStyle = '#00f2fe';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(legendX, legendY);
    ctx.lineTo(legendX + 24, legendY);
    ctx.stroke();
    ctx.fillStyle = '#f1f5f9';
    ctx.font = '12px "Plus Jakarta Sans", sans-serif';
    ctx.textAlign = 'left';
    ctx.fillText('Measured C Speedup', legendX + 32, legendY + 4);

    // Amdahl
    const legendX2 = legendX + 180;
    ctx.strokeStyle = '#a855f7';
    ctx.lineWidth = 2;
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(legendX2, legendY);
    ctx.lineTo(legendX2 + 24, legendY);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.fillText("Amdahl's Law Prediction", legendX2 + 32, legendY + 4);
  }
}

window.BenchmarkChart = BenchmarkChart;
