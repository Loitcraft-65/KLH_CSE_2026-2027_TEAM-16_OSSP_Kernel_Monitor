#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void queue_init(TaskQueue* q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->active_workers = 0;
    q->shutdown = false;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->idle_cond, NULL);
}

void queue_push(TaskQueue* q, const char* path, TaskType type, int depth) {
    Task* task = (Task*)malloc(sizeof(Task));
    if (!task) return;

    strncpy(task->path, path, MAX_PATH_LEN - 1);
    task->path[MAX_PATH_LEN - 1] = '\0';
    task->type = type;
    task->depth = depth;
    task->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->shutdown) {
        free(task);
        pthread_mutex_unlock(&q->lock);
        return;
    }

    if (q->tail) {
        q->tail->next = task;
        q->tail = task;
    } else {
        q->head = task;
        q->tail = task;
    }
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

Task* queue_pop(TaskQueue* q) {
    pthread_mutex_lock(&q->lock);

    while (q->count == 0 && !q->shutdown) {
        if (q->active_workers == 0 && q->count == 0) {
            // All workers are idle and queue is empty: search complete
            q->shutdown = true;
            pthread_cond_broadcast(&q->not_empty);
            pthread_mutex_unlock(&q->lock);
            return NULL;
        }

        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    if (q->shutdown && q->count == 0) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    Task* task = q->head;
    if (task) {
        q->head = task->next;
        if (!q->head) {
            q->tail = NULL;
        }
        q->count--;
        q->active_workers++;
    }

    pthread_mutex_unlock(&q->lock);
    return task;
}

void queue_worker_done_task(TaskQueue* q) {
    pthread_mutex_lock(&q->lock);
    if (q->active_workers > 0) {
        q->active_workers--;
    }
    if (q->active_workers == 0 && q->count == 0) {
        q->shutdown = true;
        pthread_cond_broadcast(&q->not_empty);
    }
    pthread_mutex_unlock(&q->lock);
}

void queue_shutdown(TaskQueue* q) {
    pthread_mutex_lock(&q->lock);
    q->shutdown = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

void queue_destroy(TaskQueue* q) {
    pthread_mutex_lock(&q->lock);
    q->shutdown = true;
    pthread_cond_broadcast(&q->not_empty);

    Task* cur = q->head;
    while (cur) {
        Task* next = cur->next;
        free(cur);
        cur = next;
    }
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->idle_cond);
}
