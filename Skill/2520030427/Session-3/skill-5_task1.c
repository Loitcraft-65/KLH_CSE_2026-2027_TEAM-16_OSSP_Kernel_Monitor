#include <stdio.h>
#include <string.h>

int main() {
    char input[200];

    printf("Enter a string: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    printf("\nSingle Quote Simulation\n");
    printf("Original Input: %s\n", input);
    printf("Literal Content: '%s'\n", input);
    printf("Variable expansion is ignored inside single quotes.\n");

    return 0;
}
