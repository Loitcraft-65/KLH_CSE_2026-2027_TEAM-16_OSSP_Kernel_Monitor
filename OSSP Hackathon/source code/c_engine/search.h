#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>
#include <pthread.h>
#include "queue.h"

#define MAX_EXT_LEN 256
#define MAX_SNIPPET_LEN 256
#define READ_BUFFER_SIZE 65536

typedef struct {
    char root_dir[MAX_PATH_LEN];
    char pattern[256];
    bool search_name;
    bool search_content;
    bool case_sensitive;
    bool stream_mode;
    int num_threads;
    int max_depth;
    char ext_filter[MAX_EXT_LEN];
} SearchOptions;

typedef struct {
    int thread_id;
    unsigned long files_scanned;
    unsigned long bytes_scanned;
    unsigned long matches_found;
    char current_file[MAX_PATH_LEN];
    char status[32];
    pthread_mutex_t worker_lock;
} WorkerTelemetry;

typedef struct {
    unsigned long total_files;
    unsigned long total_dirs;
    unsigned long total_bytes;
    unsigned long total_matches;
    pthread_mutex_t stats_lock;
    pthread_mutex_t print_lock;
} SearchStats;

typedef struct {
    int thread_id;
    TaskQueue* queue;
    SearchOptions* options;
    WorkerTelemetry* telemetry;
    SearchStats* stats;
} WorkerArg;

bool match_filename(const char* filename, const char* pattern, bool case_sensitive);
void search_file_content(const char* filepath, const SearchOptions* opts, int thread_id, WorkerTelemetry* telem, SearchStats* stats);
void* worker_thread_fn(void* arg);

#endif // SEARCH_H
