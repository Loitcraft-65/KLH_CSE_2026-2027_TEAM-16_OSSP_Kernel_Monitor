#include <stdio.h>
#include <pthread.h>

#define THREADS 4
#define ITERATIONS 100000

int counter = 0;
pthread_mutex_t mutex;

void *increment(void *arg)
{
    int i;

    for (i = 0; i < ITERATIONS; i++)
    {
        pthread_mutex_lock(&mutex);

        counter++;

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t threads[THREADS];
    int i;

    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < THREADS; i++)
    {
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    for (i = 0; i < THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Expected counter value: %d\n", THREADS * ITERATIONS);
    printf("Actual counter value: %d\n", counter);

    pthread_mutex_destroy(&mutex);

    return 0;
}
