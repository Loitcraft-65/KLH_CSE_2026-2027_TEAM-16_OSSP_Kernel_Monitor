#include <stdio.h>
#include <stdlib.h>

int main() {
    int i;
    int n = 5;

    printf("Dynamic Memory Allocation Demonstration\n");
    printf("----------------------------------------\n");

    int *malloc_ptr = malloc(n * sizeof(int));

    if (malloc_ptr == NULL) {
        printf("malloc failed.\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
        malloc_ptr[i] = (i + 1) * 10;
    }

    printf("\nValues using malloc():\n");

    for (i = 0; i < n; i++) {
        printf("%d ", malloc_ptr[i]);
    }

    printf("\n");

    int *calloc_ptr = calloc(n, sizeof(int));

    if (calloc_ptr == NULL) {
        printf("calloc failed.\n");
        free(malloc_ptr);
        return 1;
    }

    printf("\nValues using calloc():\n");

    for (i = 0; i < n; i++) {
        printf("%d ", calloc_ptr[i]);
    }

    printf("\n");

    n = 8;

    int *realloc_ptr = realloc(malloc_ptr, n * sizeof(int));

    if (realloc_ptr == NULL) {
        printf("realloc failed.\n");
        free(malloc_ptr);
        free(calloc_ptr);
        return 1;
    }

    malloc_ptr = realloc_ptr;

    for (i = 5; i < n; i++) {
        malloc_ptr[i] = (i + 1) * 10;
    }

    printf("\nValues after realloc():\n");

    for (i = 0; i < n; i++) {
        printf("%d ", malloc_ptr[i]);
    }

    printf("\n");

    free(malloc_ptr);
    free(calloc_ptr);

    printf("\nMemory successfully released using free().\n");

    return 0;
}
