#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_PATH_LEN 1024

typedef enum {
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
    pthread_cond_t idle_cond;
} TaskQueue;

void queue_init(TaskQueue* q);
void queue_push(TaskQueue* q, const char* path, TaskType type, int depth);
Task* queue_pop(TaskQueue* q);
void queue_worker_idle(TaskQueue* q);
void queue_worker_busy(TaskQueue* q);
void queue_worker_done_task(TaskQueue* q);
void queue_shutdown(TaskQueue* q);
void queue_destroy(TaskQueue* q);

#endif // QUEUE_H
