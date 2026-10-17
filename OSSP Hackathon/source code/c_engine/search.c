#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// Escape string for valid JSON output
void escape_json(const char* src, char* dest, size_t max_dest) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 5 < max_dest; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"') {
            dest[j++] = '\\';
            dest[j++] = '"';
        } else if (c == '\\') {
            dest[j++] = '/'; // Convert backslash to forward slash for clean cross-platform JSON
        } else if (c == '\n') {
            dest[j++] = ' ';
        } else if (c == '\r') {
            dest[j++] = ' ';
        } else if (c == '\t') {
            dest[j++] = ' ';
        } else if (c < 32 || c > 126) {
            // Replace non-printable ASCII or control characters
            dest[j++] = ' ';
        } else {
            dest[j++] = c;
        }
    }
    dest[j] = '\0';
}

// Case-insensitive substring search helper
static const char* strcasestr_custom(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    for (; *haystack; haystack++) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char* h = haystack;
            const char* n = needle;
            while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
                h++;
                n++;
            }
            if (!*n) return haystack;
        }
    }
    return NULL;
}

// Check if filename matches extension filter (e.g. ".c,.h,.py,.txt")
static bool matches_extension_filter(const char* filename, const char* ext_filter) {
    if (!ext_filter || ext_filter[0] == '\0') return true;

    const char* dot = strrchr(filename, '.');
    if (!dot) return false;

    char ext_lower[64];
    size_t k = 0;
    while (dot[k] && k < 63) {
        ext_lower[k] = (char)tolower((unsigned char)dot[k]);
        k++;
    }
    ext_lower[k] = '\0';

    char filter_copy[MAX_EXT_LEN];
    strncpy(filter_copy, ext_filter, MAX_EXT_LEN - 1);
    filter_copy[MAX_EXT_LEN - 1] = '\0';

    char* token = strtok(filter_copy, ",; ");
    while (token) {
        char clean_tok[64];
        size_t c = 0;
        if (token[0] != '.') {
            clean_tok[c++] = '.';
        }
        for (size_t t = 0; token[t] && c < 63; t++) {
            clean_tok[c++] = (char)tolower((unsigned char)token[t]);
        }
        clean_tok[c] = '\0';

        if (strcmp(ext_lower, clean_tok) == 0) {
            return true;
        }
        token = strtok(NULL, ",; ");
    }
    return false;
}

// Match filename against pattern
bool match_filename(const char* filename, const char* pattern, bool case_sensitive) {
    if (!pattern || pattern[0] == '\0') return true;

    if (case_sensitive) {
        return (strstr(filename, pattern) != NULL);
    } else {
        return (strcasestr_custom(filename, pattern) != NULL);
    }
}

// Search inside file content
void search_file_content(const char* filepath, const SearchOptions* opts, int thread_id, WorkerTelemetry* telem, SearchStats* stats) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) return;

    // Check first 1024 bytes for binary null characters
    char sample[1024];
    size_t bytes_read = fread(sample, 1, sizeof(sample), fp);
    for (size_t i = 0; i < bytes_read; i++) {
        if (sample[i] == '\0') {
            fclose(fp);
            return; // Skip binary files
        }
    }
    rewind(fp);

    char line_buf[2048];
    unsigned long line_num = 0;
    char escaped_path[MAX_PATH_LEN * 2];
    char escaped_snippet[MAX_SNIPPET_LEN * 2];

    escape_json(filepath, escaped_path, sizeof(escaped_path));

    while (fgets(line_buf, sizeof(line_buf), fp)) {
        line_num++;
        size_t len = strlen(line_buf);
        telem->bytes_scanned += len;

        const char* match = NULL;
        if (opts->case_sensitive) {
            match = strstr(line_buf, opts->pattern);
        } else {
            match = strcasestr_custom(line_buf, opts->pattern);
        }

        if (match) {
            pthread_mutex_lock(&telem->worker_lock);
            telem->matches_found++;
            pthread_mutex_unlock(&telem->worker_lock);

            pthread_mutex_lock(&stats->stats_lock);
            stats->total_matches++;
            pthread_mutex_unlock(&stats->stats_lock);

            if (opts->stream_mode) {
                // Trim snippet to 180 chars max
                char snippet[MAX_SNIPPET_LEN];
                strncpy(snippet, line_buf, MAX_SNIPPET_LEN - 1);
                snippet[MAX_SNIPPET_LEN - 1] = '\0';
                escape_json(snippet, escaped_snippet, sizeof(escaped_snippet));

                pthread_mutex_lock(&stats->print_lock);
                printf("{\"type\":\"match\",\"thread_id\":%d,\"path\":\"%s\",\"line\":%lu,\"snippet\":\"%s\",\"match_type\":\"content\"}\n",
                       thread_id, escaped_path, line_num, escaped_snippet);
                fflush(stdout);
                pthread_mutex_unlock(&stats->print_lock);
            }
        }
    }

    fclose(fp);
}

// Worker thread entry function
void* worker_thread_fn(void* arg) {
    WorkerArg* warg = (WorkerArg*)arg;
    int tid = warg->thread_id;
    TaskQueue* queue = warg->queue;
    SearchOptions* opts = warg->options;
    WorkerTelemetry* telem = warg->telemetry;
    SearchStats* stats = warg->stats;

    char escaped_path[MAX_PATH_LEN * 2];

    while (1) {
        Task* task = queue_pop(queue);
        if (!task) {
            // Queue empty and all workers done
            break;
        }

        pthread_mutex_lock(&telem->worker_lock);
        strncpy(telem->current_file, task->path, MAX_PATH_LEN - 1);
        telem->current_file[MAX_PATH_LEN - 1] = '\0';
        strncpy(telem->status, "SCANNING", 31);
        pthread_mutex_unlock(&telem->worker_lock);

        if (task->type == TASK_DIRECTORY) {
            pthread_mutex_lock(&stats->stats_lock);
            stats->total_dirs++;
            pthread_mutex_unlock(&stats->stats_lock);

            DIR* dir = opendir(task->path);
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != NULL) {
                    // Ignore current and parent dir
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }
                    // Skip VCS and package caches
                    if (strcmp(entry->d_name, ".git") == 0 ||
                        strcmp(entry->d_name, ".svn") == 0 ||
                        strcmp(entry->d_name, "node_modules") == 0 ||
                        strcmp(entry->d_name, "__pycache__") == 0 ||
                        strcmp(entry->d_name, ".vscode") == 0) {
                        continue;
                    }

                    char child_path[MAX_PATH_LEN];
                    snprintf(child_path, sizeof(child_path), "%s/%s", task->path, entry->d_name);

                    struct stat st;
                    if (stat(child_path, &st) == 0) {
                        if (S_ISDIR(st.st_mode)) {
                            if (opts->max_depth <= 0 || task->depth + 1 <= opts->max_depth) {
                                queue_push(queue, child_path, TASK_DIRECTORY, task->depth + 1);
                            }
                        } else if (S_ISREG(st.st_mode)) {
                            queue_push(queue, child_path, TASK_FILE, task->depth + 1);
                        }
                    }
                }
                closedir(dir);
            }
        } else if (task->type == TASK_FILE) {
            pthread_mutex_lock(&stats->stats_lock);
            stats->total_files++;
            pthread_mutex_unlock(&stats->stats_lock);

            pthread_mutex_lock(&telem->worker_lock);
            telem->files_scanned++;
            strncpy(telem->status, "SEARCHING", 31);
            pthread_mutex_unlock(&telem->worker_lock);

            // Extract file name
            const char* fname = strrchr(task->path, '/');
            if (!fname) fname = strrchr(task->path, '\\');
            fname = fname ? fname + 1 : task->path;

            // Check extension filter
            if (matches_extension_filter(fname, opts->ext_filter)) {
                // Filename search
                if (opts->search_name && match_filename(fname, opts->pattern, opts->case_sensitive)) {
                    pthread_mutex_lock(&telem->worker_lock);
                    telem->matches_found++;
                    pthread_mutex_unlock(&telem->worker_lock);

                    pthread_mutex_lock(&stats->stats_lock);
                    stats->total_matches++;
                    pthread_mutex_unlock(&stats->stats_lock);

                    if (opts->stream_mode) {
                        escape_json(task->path, escaped_path, sizeof(escaped_path));
                        pthread_mutex_lock(&stats->print_lock);
                        printf("{\"type\":\"match\",\"thread_id\":%d,\"path\":\"%s\",\"line\":0,\"snippet\":\"Filename matched: %s\",\"match_type\":\"name\"}\n",
                               tid, escaped_path, fname);
                        fflush(stdout);
                        pthread_mutex_unlock(&stats->print_lock);
                    }
                }

                // Content search
                if (opts->search_content && opts->pattern[0] != '\0') {
                    search_file_content(task->path, opts, tid, telem, stats);
                }
            }
        }

        queue_worker_done_task(queue);
        free(task);
    }

    pthread_mutex_lock(&telem->worker_lock);
    strncpy(telem->status, "DONE", 31);
    pthread_mutex_unlock(&telem->worker_lock);

    return NULL;
}
