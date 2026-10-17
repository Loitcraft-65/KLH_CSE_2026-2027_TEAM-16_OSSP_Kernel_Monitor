import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import parse_xml, OxmlElement
from docx.oxml.ns import nsdecls, qn
import os

def set_cell_background(cell, fill_hex):
    """Sets background color of a table cell."""
    tcPr = cell._tc.get_or_add_tcPr()
    shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    tcPr.append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    """Sets cell padding in dxa."""
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = parse_xml(
        f'<w:tcMar {nsdecls("w")}>'
        f'<w:top w:w="{top}" w:type="dxa"/>'
        f'<w:bottom w:w="{bottom}" w:type="dxa"/>'
        f'<w:left w:w="{left}" w:type="dxa"/>'
        f'<w:right w:w="{right}" w:type="dxa"/>'
        f'</w:tcMar>'
    )
    tcPr.append(tcMar)

def create_document():
    doc = docx.Document()

    # Set Margins
    for section in doc.sections:
        section.top_margin = Inches(1.0)
        section.bottom_margin = Inches(1.0)
        section.left_margin = Inches(1.0)
        section.right_margin = Inches(1.0)

    # Styles
    normal_style = doc.styles['Normal']
    normal_style.font.name = 'Calibri'
    normal_style.font.size = Pt(11)
    normal_style.font.color.rgb = RGBColor(30, 41, 59) # Slate 800

    # -------------------------------------------------------------
    # COVER PAGE
    # -------------------------------------------------------------
    title_p = doc.add_paragraph()
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_p.paragraph_format.space_before = Pt(40)
    title_p.paragraph_format.space_after = Pt(8)

    badge_run = title_p.add_run("OSSP HACKATHON 2026 | TECHNICAL REPORT\n")
    badge_run.font.name = 'Arial'
    badge_run.font.size = Pt(11)
    badge_run.font.bold = True
    badge_run.font.color.rgb = RGBColor(2, 132, 199) # Accent Blue

    title_run = title_p.add_run("AeroSearch: High-Performance Parallel File Search Engine\n")
    title_run.font.name = 'Arial'
    title_run.font.size = Pt(24)
    title_run.font.bold = True
    title_run.font.color.rgb = RGBColor(15, 23, 42) # Deep Navy

    sub_run = title_p.add_run("Dynamic Producer-Consumer Concurrency using POSIX Threads & Real-Time Telemetry")
    sub_run.font.name = 'Calibri'
    sub_run.font.size = Pt(13)
    sub_run.font.italic = True
    sub_run.font.color.rgb = RGBColor(100, 116, 139) # Slate 500

    doc.add_paragraph().paragraph_format.space_after = Pt(20)

    # Team Box Table
    team_table = doc.add_table(rows=5, cols=3)
    team_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    team_table.autofit = False

    col_widths = [Inches(2.0), Inches(1.8), Inches(2.6)]
    for row in team_table.rows:
        for idx, width in enumerate(col_widths):
            row.cells[idx].width = width

    # Header Row
    hdr_cells = team_table.rows[0].cells
    hdr_cells[0].merge(hdr_cells[2])
    hdr_p = hdr_cells[0].paragraphs[0]
    hdr_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    hdr_run = hdr_p.add_run("PROJECT SUBMISSION: TEAM - 16")
    hdr_run.font.name = 'Arial'
    hdr_run.font.bold = True
    hdr_run.font.size = Pt(12)
    hdr_run.font.color.rgb = RGBColor(255, 255, 255)
    set_cell_background(hdr_cells[0], "0F172A")
    set_cell_margins(hdr_cells[0], top=140, bottom=140)

    # Table Column Headers
    cols = team_table.rows[1].cells
    col_titles = ["Student Name", "Roll / Reg Number", "Assigned Role"]
    for i, title in enumerate(col_titles):
        p = cols[i].paragraphs[0]
        r = p.add_run(title)
        r.font.name = 'Arial'
        r.font.bold = True
        r.font.size = Pt(10)
        r.font.color.rgb = RGBColor(15, 23, 42)
        set_cell_background(cols[i], "E2E8F0")
        set_cell_margins(cols[i], top=100, bottom=100)

    # Member Rows
    members_data = [
        ("Lohith Sanapala", "2520030023", "C Core Multi-threading & Task Queue"),
        ("Saahas Vaddi", "2520030256", "FastAPI Bridge & Amdahl Benchmarking"),
        ("Rapaka Rithwik", "2520030427", "Frontend HUD & Test Data Generator")
    ]

    for row_idx, data in enumerate(members_data, start=2):
        row_cells = team_table.rows[row_idx].cells
        bg_color = "F8FAFC" if row_idx % 2 == 0 else "FFFFFF"
        for col_idx, text in enumerate(data):
            p = row_cells[col_idx].paragraphs[0]
            r = p.add_run(text)
            r.font.size = Pt(10)
            if col_idx == 0:
                r.font.bold = True
            set_cell_background(row_cells[col_idx], bg_color)
            set_cell_margins(row_cells[col_idx], top=80, bottom=80)

    # Metadata Paragraph
    meta_p = doc.add_paragraph()
    meta_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    meta_p.paragraph_format.space_before = Pt(36)
    m_run = meta_p.add_run(
        "Topic: Parallel File Search using Threads\n"
        "Core Basis: Native C (POSIX Threads / GCC 16.2.0)\n"
        "Date of Submission: October 2026"
    )
    m_run.font.size = Pt(10)
    m_run.font.color.rgb = RGBColor(100, 116, 139)

    doc.add_page_break()

    # Helper function for Section Headings
    def add_heading_1(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(20)
        p.paragraph_format.space_after = Pt(8)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.name = 'Arial'
        run.font.size = Pt(16)
        run.font.bold = True
        run.font.color.rgb = RGBColor(15, 23, 42)
        return p

    def add_heading_2(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(14)
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.name = 'Arial'
        run.font.size = Pt(13)
        run.font.bold = True
        run.font.color.rgb = RGBColor(2, 132, 199)
        return p

    def add_code_block(code_text):
        tbl = doc.add_table(rows=1, cols=1)
        tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = tbl.rows[0].cells[0]
        cell.width = Inches(6.5)
        set_cell_background(cell, "0F172A")
        set_cell_margins(cell, top=100, bottom=100, left=150, right=150)
        p = cell.paragraphs[0]
        p.paragraph_format.space_after = Pt(0)
        r = p.add_run(code_text.strip())
        r.font.name = 'Consolas'
        r.font.size = Pt(9.5)
        r.font.color.rgb = RGBColor(241, 245, 249)
        doc.add_paragraph().paragraph_format.space_after = Pt(6)

    def add_callout(text):
        tbl = doc.add_table(rows=1, cols=1)
        tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = tbl.rows[0].cells[0]
        cell.width = Inches(6.5)
        set_cell_background(cell, "F0F9FF")
        set_cell_margins(cell, top=100, bottom=100, left=150, right=150)
        # Left border blue
        tcPr = cell._tc.get_or_add_tcPr()
        tcBorders = parse_xml(
            f'<w:tcBorders {nsdecls("w")}>'
            f'<w:left w:val="single" w:sz="24" w:space="0" w:color="0284C7"/>'
            f'<w:top w:val="none"/>'
            f'<w:right w:val="none"/>'
            f'<w:bottom w:val="none"/>'
            f'</w:tcBorders>'
        )
        tcPr.append(tcBorders)
        p = cell.paragraphs[0]
        p.paragraph_format.space_after = Pt(0)
        r = p.add_run(text)
        r.font.name = 'Calibri'
        r.font.size = Pt(10.5)
        r.font.color.rgb = RGBColor(12, 74, 110)
        doc.add_paragraph().paragraph_format.space_after = Pt(6)

    # -------------------------------------------------------------
    # 1. EXECUTIVE SUMMARY
    # -------------------------------------------------------------
    add_heading_1("1. Executive Summary")
    doc.add_paragraph(
        "In modern computing environments, storage capacities and directory depths continue to expand exponentially. "
        "However, traditional search utilities (such as naive single-threaded find or grep implementations) process files "
        "sequentially, leading to severe CPU underutilization and excessive wait times on multi-core workstations."
    )
    doc.add_paragraph(
        "AeroSearch is an end-to-end, high-concurrency file traversal and full-text pattern matching platform designed for "
        "the OSSP Hackathon. The computational engine is engineered entirely in C using native POSIX Threads (pthreads). "
        "To eliminate thread starvation caused by asymmetrical folder hierarchies, AeroSearch introduces a dynamic work-stealing "
        "task queue coordinated by mutual exclusion locks (pthread_mutex_t) and condition variables (pthread_cond_t)."
    )
    add_callout(
        "Key Metric Achieved: Across an 800-file nested benchmark suite, AeroSearch scales from 99.87 ms (1 Thread) down to "
        "35.67 ms (4 Threads), achieving an empirical 2.80x speedup and processing over 22,400 files/second before saturating "
        "storage I/O bus bandwidth."
    )

    # -------------------------------------------------------------
    # 2. PROBLEM STATEMENT & MOTIVATION
    # -------------------------------------------------------------
    add_heading_1("2. Problem Statement & Motivation")
    doc.add_paragraph(
        "Single-threaded directory search algorithms suffer from three fundamental architectural bottlenecks:"
    )
    
    p = doc.add_paragraph()
    p.paragraph_format.left_indent = Inches(0.25)
    r = p.add_run("1. I/O Blocking: ")
    r.bold = True
    p.add_run("When a single thread requests file metadata or contents from disk, the thread blocks in an OS wait state. Multi-core microprocessors remain idle while storage controllers transfer data into memory buffers.")

    p2 = doc.add_paragraph()
    p2.paragraph_format.left_indent = Inches(0.25)
    r = p2.add_run("2. Asymmetrical Directory Trees: ")
    r.bold = True
    p2.add_run("Real-world file systems exhibit severe structural skew. For instance, a software development repository may contain 12,000 files in a node_modules or build folder, but only 4 files in a sibling config directory. Naive static partitioning results in one thread laboring while others sit starved.")

    p3 = doc.add_paragraph()
    p3.paragraph_format.left_indent = Inches(0.25)
    r = p3.add_run("3. Lack of Concurrency Observability: ")
    r.bold = True
    p3.add_run("CLI search tools typically run as opaque processes without providing visibility into core load distribution, worker state transitions, or throughput dynamics.")

    # -------------------------------------------------------------
    # 3. SYSTEM ARCHITECTURE
    # -------------------------------------------------------------
    add_heading_1("3. System Architecture")
    doc.add_paragraph(
        "AeroSearch is architected into three decoupled tiers: the C Computational Core, the FastAPI IPC Bridge, "
        "and the Cyber-Industrial Web Dashboard."
    )

    add_code_block(
"""+-------------------------------------------------------------------------+
|                        Frontend (Browser UI)                            |
|   - Real-time Concurrency HUD (Active Worker Cards, File Counters)      |
|   - Results Explorer (Filename & Full-Text Content Matches)             |
|   - Canvas 2D Benchmark Visualizer (Speedup vs. Amdahl's Law Curve)     |
+-----------------------------------^-------------------------------------+
                                    | WebSocket (JSON stream, 16-30 FPS)
+-----------------------------------v-------------------------------------+
|                      FastAPI Server (Python 3.14)                       |
|   - Subprocess Pipe Bridge (c_bridge.py)                                |
|   - Concurrency Benchmark Suite Runner (benchmark.py)                   |
|   - Procedural Test Dataset Generator (data_gen.py)                     |
+-----------------------------------^-------------------------------------+
                                    | Async stdio (Non-blocking pipes)
+-----------------------------------v-------------------------------------+
|              Native C Core Search Engine (POSIX Threads)                |
|   +-----------------------------------------------------------------+   |
|   |                  Thread-Safe Task Queue                         |   |
|   |         (pthread_mutex_t lock, pthread_cond_t not_empty)        |   |
|   +-------------------------------+---------------------------------+   |
|                                   | Pull Tasks                          |
|         +-------------------------+-------------------------+           |
|         v                         v                         v           |
|    Worker Thread #0          Worker Thread #1          Worker Thread #N |
|   [Directory Traversal]     [Directory Traversal]     [Content Grep]    |
+-------------------------------------------------------------------------+"""
    )

    doc.add_paragraph(
        "1. Computational Core (C Engine): Statically compiled (parallel_search.exe) with GCC 16.2.0 (-O3 -static). "
        "Directly invokes POSIX directory routines (opendir, readdir) and buffered file reading without runtime overhead.\n"
        "2. Orchestration Bridge (FastAPI): Manages subprocess lifecycle via asyncio.create_subprocess_exec, reading "
        "atomic JSON telemetry emitted by C stdout and streaming it across WebSockets.\n"
        "3. Presentation Dashboard: Built with HTML5, Vanilla CSS3, and Canvas 2D. Features pulsing thread indicators, "
        "file match context previews, and Amdahl's Law scaling curves."
    )

    # -------------------------------------------------------------
    # 4. CONCURRENCY & OS SYNCHRONIZATION
    # -------------------------------------------------------------
    add_heading_1("4. Concurrency Model & OS Synchronization")
    add_heading_2("4.1 Dynamic Work-Stealing Task Queue")
    doc.add_paragraph(
        "The queue is implemented as a thread-safe linked list in c_engine/queue.c. Tasks encapsulate either a "
        "TASK_DIRECTORY or a TASK_FILE:"
    )

    add_code_block(
"""typedef enum {
    TASK_DIRECTORY,
    TASK_FILE
} TaskType;

typedef struct Task {
    char path[MAX_PATH_LEN];
    TaskType type;
    int depth;
    struct Task* next;
} Task;

typedef struct {
    Task* head;
    Task* tail;
    int count;
    int active_workers;
    bool shutdown;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} TaskQueue;"""
    )

    add_heading_2("4.2 Mutex Locking & Condition Scheduling")
    doc.add_paragraph(
        "Thread synchronization strictly enforces the following design principles:\n"
        "• Minimal Critical Sections: pthread_mutex_lock(&q->lock) is held exclusively during queue pointer adjustments and counter increments.\n"
        "• Zero-Busy-Wait Sleep: When the queue is empty, workers call pthread_cond_wait(&q->not_empty, &q->lock). The operating system puts the thread to sleep until a sibling thread enqueues tasks and signals pthread_cond_signal.\n"
        "• Deadlock Avoidance & Clean Termination: In parallel tree traversal, an empty queue does not mean searching is finished, because another thread may still be opening a directory that will produce dozens of files. The system terminates cleanly if and only if:\n"
        "        (queue_count == 0) and (active_workers == 0)\n"
        "When this condition is reached, q->shutdown = true is set and pthread_cond_broadcast awakens all waiting threads to exit cleanly."
    )

    # -------------------------------------------------------------
    # 5. EXPERIMENTAL BENCHMARKS & AMDAHL'S LAW
    # -------------------------------------------------------------
    add_heading_1("5. Experimental Benchmarks & Amdahl's Law")
    doc.add_paragraph(
        "Amdahl's Law defines the theoretical maximum speedup S(n) of an application scaling across n threads:"
    )
    
    add_callout(
        "Amdahl's Law Formula:\n"
        "S(n) = 1 / ((1 - P) + (P / n))\n\n"
        "Where P represents the parallel fraction (file reading and pattern matching ~0.88), "
        "and (1 - P) represents the serial fraction (thread spawning, queue locking, console flushing ~0.12)."
    )

    # Benchmark Table
    bench_table = doc.add_table(rows=6, cols=6)
    bench_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    bench_table.autofit = False

    b_widths = [Inches(1.1), Inches(1.1), Inches(1.1), Inches(1.1), Inches(1.1), Inches(1.2)]
    for row in bench_table.rows:
        for idx, width in enumerate(b_widths):
            row.cells[idx].width = width

    # Benchmark Headers
    b_headers = ["Threads (n)", "Time (Tn)", "Speedup (Sn)", "Efficiency", "Amdahl Speedup", "Throughput"]
    for i, h in enumerate(b_headers):
        cell = bench_table.rows[0].cells[i]
        p = cell.paragraphs[0]
        r = p.add_run(h)
        r.font.name = 'Arial'
        r.font.bold = True
        r.font.size = Pt(9.5)
        r.font.color.rgb = RGBColor(255, 255, 255)
        set_cell_background(cell, "0F172A")
        set_cell_margins(cell, top=100, bottom=100)

    # Benchmark Data
    b_rows = [
        ("1 Thread", "99.87 ms", "1.00x", "100.0%", "1.00x", "8,010 files/s"),
        ("2 Threads", "64.04 ms", "1.56x", "78.0%", "1.79x", "12,492 files/s"),
        ("4 Threads", "35.67 ms", "2.80x", "70.0%", "2.94x", "22,427 files/s"),
        ("8 Threads", "46.34 ms", "2.16x", "27.0%", "4.35x", "17,263 files/s"),
        ("16 Threads", "42.93 ms", "2.33x", "14.6%", "5.71x", "18,635 files/s")
    ]

    for row_idx, data in enumerate(b_rows, start=1):
        bg_color = "F1F5F9" if row_idx % 2 == 0 else "FFFFFF"
        for col_idx, val in enumerate(data):
            cell = bench_table.rows[row_idx].cells[col_idx]
            p = cell.paragraphs[0]
            r = p.add_run(val)
            r.font.size = Pt(9.5)
            if col_idx == 0 or col_idx == 2:
                r.font.bold = True
            if row_idx == 3: # 4 threads highlight
                r.font.bold = True
                r.font.color.rgb = RGBColor(2, 132, 199)
            set_cell_background(cell, bg_color)
            set_cell_margins(cell, top=80, bottom=80)

    doc.add_paragraph().paragraph_format.space_after = Pt(8)
    doc.add_paragraph(
        "Scaling Analysis: Peak throughput is reached at 4 worker threads, processing 22,427 files/second. "
        "Beyond 8 threads, operating system thread context-switching overhead and disk I/O head contention limit further "
        "improvements, providing an empirical demonstration of storage bus saturation."
    )

    # -------------------------------------------------------------
    # 6. ENGINEERING CHALLENGES & SOLUTIONS
    # -------------------------------------------------------------
    add_heading_1("6. Engineering Challenges & Solutions")
    
    chal_table = doc.add_table(rows=4, cols=3)
    chal_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    chal_table.autofit = False

    c_widths = [Inches(1.8), Inches(2.2), Inches(2.5)]
    for row in chal_table.rows:
        for idx, width in enumerate(c_widths):
            row.cells[idx].width = width

    c_headers = ["Challenge", "Root Cause", "Engineering Solution"]
    for i, h in enumerate(c_headers):
        cell = chal_table.rows[0].cells[i]
        p = cell.paragraphs[0]
        r = p.add_run(h)
        r.font.name = 'Arial'
        r.font.bold = True
        r.font.size = Pt(9.5)
        r.font.color.rgb = RGBColor(255, 255, 255)
        set_cell_background(cell, "0F172A")
        set_cell_margins(cell, top=100, bottom=100)

    c_rows = [
        ("Missing DLLs on Windows", "MinGW dynamically links libwinpthread-1.dll by default.", "Statically linked runtime libraries (-static) to produce a 100% self-contained binary."),
        ("Path Backslash Escaping", "Raw Windows backslashes in paths break JSON parsers.", "Implemented in-place C escape_json() normalizing backslashes to forward slashes."),
        ("Browser Event Flooding", "Emitting every scanned file over WebSockets froze browser rendering.", "Implemented a dedicated 60ms telemetry reporter thread in C to pulse aggregated metrics.")
    ]

    for row_idx, data in enumerate(c_rows, start=1):
        bg_color = "F8FAFC" if row_idx % 2 == 0 else "FFFFFF"
        for col_idx, val in enumerate(data):
            cell = chal_table.rows[row_idx].cells[col_idx]
            p = cell.paragraphs[0]
            r = p.add_run(val)
            r.font.size = Pt(9.5)
            if col_idx == 0:
                r.font.bold = True
            set_cell_background(cell, bg_color)
            set_cell_margins(cell, top=80, bottom=80)

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # 7. EXECUTION & CLI MANUAL
    # -------------------------------------------------------------
    add_heading_1("7. Execution Manual")
    doc.add_paragraph("Launch unified full-stack application (opens browser at http://127.0.0.1:8000/):")
    add_code_block("python run.py")

    doc.add_paragraph("Run standalone C engine in terminal:")
    add_code_block(
"./c_engine/parallel_search.exe -d ./test_dataset -p \"HACKATHON_CHAMPION_2026\" -t 8\n"
"./c_engine/parallel_search.exe -d ./test_dataset -p \"queue\" -t 4 --stream"
    )

    # -------------------------------------------------------------
    # 8. CONCLUSION & SIGN-OFF
    # -------------------------------------------------------------
    add_heading_1("8. Conclusion & Sign-Off")
    doc.add_paragraph(
        "AeroSearch demonstrates that native multi-threading in C using POSIX Threads yields superior speedup "
        "(up to 2.80x over single-threaded search) while dynamic task queues effectively prevent thread starvation. "
        "The project integrates low-level systems programming with real-time web telemetry to provide an accessible, "
        "observable, and high-performance file search architecture."
    )

    sign_p = doc.add_paragraph()
    sign_p.paragraph_format.space_before = Pt(24)
    sign_p.add_run(
        "Project Submitted by Team - 16:\n"
        "• Lohith Sanapala (2520030023)\n"
        "• Saahas Vaddi (2520030256)\n"
        "• Rapaka Rithwik (2520030427)\n"
    )

    # Save Document
    target_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "PROJECT_DOCUMENTATION.docx"))
    doc.save(target_path)
    print(f"[SUCCESS] Document generated at: {target_path}")

    # Also save a copy inside docs/ folder
    docs_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "docs"))
    os.makedirs(docs_dir, exist_ok=True)
    docs_copy_path = os.path.join(docs_dir, "PROJECT_DOCUMENTATION.docx")
    doc.save(docs_copy_path)
    print(f"[SUCCESS] Copy saved at: {docs_copy_path}")

if __name__ == "__main__":
    create_document()
