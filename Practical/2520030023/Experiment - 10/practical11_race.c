#include <stdio.h>
#include <pthread.h>

#define THREADS 4
#define ITERATIONS 100000

int counter = 0;

void *increment(void *arg)
{
    int i;

    for (i = 0; i < ITERATIONS; i++)
    {
        counter++;
    }

    return NULL;
}

int main()
{
    pthread_t threads[THREADS];
    int i;

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

    return 0;
}
