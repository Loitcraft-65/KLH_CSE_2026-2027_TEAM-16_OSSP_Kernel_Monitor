#include <stdio.h>
#include <stdlib.h>

int global_var = 100;

void code_function() {
    printf("Code address   : %p\n", (void *)code_function);
}

int main() {
    static int static_var = 200;
    int stack_var = 300;
    int *heap_var = malloc(sizeof(int));

    if (heap_var == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    *heap_var = 400;

    printf("Memory Address Demonstration\n");
    printf("----------------------------\n");

    code_function();

    printf("Global address : %p\n", (void *)&global_var);
    printf("Static address : %p\n", (void *)&static_var);
    printf("Heap address   : %p\n", (void *)heap_var);
    printf("Stack address  : %p\n", (void *)&stack_var);

    printf("\nValues\n");
    printf("Global = %d\n", global_var);
    printf("Static = %d\n", static_var);
    printf("Heap   = %d\n", *heap_var);
    printf("Stack  = %d\n", stack_var);

    free(heap_var);

    return 0;
}
