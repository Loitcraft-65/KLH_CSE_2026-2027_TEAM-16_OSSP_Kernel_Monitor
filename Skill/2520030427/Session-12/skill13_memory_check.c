#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *message = malloc(100);

    if (message == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    strcpy(message, "OSSP memory checking experiment");

    printf("%s\n", message);

    free(message);

    return 0;
}
