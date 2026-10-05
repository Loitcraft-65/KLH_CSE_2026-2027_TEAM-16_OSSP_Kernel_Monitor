#include <stdio.h>

int main() {
    int number;

    printf("Enter a positive integer: ");

    if (scanf("%d", &number) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    if (number <= 0) {
        printf("Invalid input. Number must be positive.\n");
        return 1;
    }

    printf("Valid input: %d\n", number);

    return 0;
}
