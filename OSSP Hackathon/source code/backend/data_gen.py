import os
import random
import string
from typing import Dict, Any

CODE_SNIPPETS = [
    "int thread_worker_execute(void* task) {\n    printf(\"Executing parallel worker\\n\");\n    return 0;\n}\n",
    "def calculate_speedup(t1, tn):\n    return t1 / tn if tn > 0 else 0.0\n",
    "// High concurrency mutex synchronization block\npthread_mutex_lock(&lock);\nshared_resource++;\npthread_mutex_unlock(&lock);\n",
    "const handleSearchQuery = async (query) => {\n  const res = await fetch(`/api/search?q=${query}`);\n  return res.json();\n};\n",
    "{\"service\": \"concurrency_engine\", \"status\": \"healthy\", \"workers\": 16, \"load\": 0.42}\n",
    "# Architecture Specification\nThis module implements a dynamic work-stealing thread queue.\n"
]

LOG_TEMPLATES = [
    "[INFO] [Thread-{tid}] Successfully processed chunk {idx} in {lat}ms\n",
    "[DEBUG] [WorkerPool] Active threads: {threads}, Pending queue tasks: {tasks}\n",
    "[WARN] [I/O-Scheduler] Cache hit ratio 94.2% on block #{idx}\n",
    "[INFO] [Telemetry] Core temperature normal, CPU load across cores balanced\n"
]

SPECIAL_TOKENS = [
    "HACKATHON_CHAMPION_2026",
    "DATABASE_DEADLOCK_DETECTED",
    "PARALLEL_THREAD_WORKER_SIG_ALPHA",
    "CRITICAL_SECURITY_LEAK_TOKEN_99",
    "HIGH_THROUGHPUT_PIPELINE_OK",
    "QUANTUM_CONCURRENCY_KEY",
    "OSSP_THREAD_BENCHMARK_TARGET"
]

SUBDIRS = [
    "src/core",
    "src/utils",
    "src/networking",
    "src/concurrency",
    "docs/architecture",
    "docs/benchmarks",
    "logs/server",
    "logs/audit",
    "config/environments",
    "data/analytics"
]

def generate_mock_dataset(target_dir: str, file_count: int = 600) -> Dict[str, Any]:
    os.makedirs(target_dir, exist_ok=True)
    created_files = 0
    token_occurrences = {tok: 0 for tok in SPECIAL_TOKENS}

    # Ensure all subdirectories exist
    for sub in SUBDIRS:
        os.makedirs(os.path.join(target_dir, sub), exist_ok=True)

    extensions = [".c", ".h", ".py", ".js", ".json", ".log", ".md", ".txt"]

    # Randomly inject special tokens into specific files
    needle_targets = set(random.sample(range(file_count), min(len(SPECIAL_TOKENS) * 4, file_count)))
    token_cycle = SPECIAL_TOKENS * 10

    for i in range(file_count):
        sub = random.choice(SUBDIRS)
        ext = random.choice(extensions)
        filename = f"module_{i:04d}_{''.join(random.choices(string.ascii_lowercase, k=4))}{ext}"
        filepath = os.path.join(target_dir, sub, filename)

        # Generate content
        content_lines = []
        for _ in range(random.randint(5, 30)):
            if ext in [".c", ".h", ".py", ".js"]:
                content_lines.append(random.choice(CODE_SNIPPETS))
            elif ext == ".log":
                content_lines.append(
                    random.choice(LOG_TEMPLATES).format(
                        tid=random.randint(0, 15),
                        idx=random.randint(100, 9999),
                        lat=random.randint(1, 45),
                        threads=random.choice([2, 4, 8, 16]),
                        tasks=random.randint(0, 120)
                    )
                )
            else:
                content_lines.append(f"Section {random.randint(1, 100)}: Simulated documentation and data points.\n")

        # Inject needle token if selected
        if i in needle_targets:
            token = token_cycle[i % len(token_cycle)]
            insert_pos = random.randint(0, len(content_lines))
            content_lines.insert(insert_pos, f"\n// TARGET NEEDLE: {token} [INDEX {i}]\n")
            token_occurrences[token] += 1

        with open(filepath, "w", encoding="utf-8") as f:
            f.writelines(content_lines)

        created_files += 1

    return {
        "status": "success",
        "directory": target_dir.replace("\\", "/"),
        "files_created": created_files,
        "subdirectories": len(SUBDIRS),
        "injected_tokens": token_occurrences
    }
