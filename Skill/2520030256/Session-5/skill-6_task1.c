#include <stdio.h>
#include <string.h>

int main() {
    char input[200];

    printf("Enter text with escaped characters: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    printf("\nOriginal Input:\n");
    printf("%s\n", input);

    printf("\nProcessed Input:\n");

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\\' && input[i + 1] != '\0') {
            i++;
            printf("%c", input[i]);
        } else {
            printf("%c", input[i]);
        }
    }

    printf("\n");

    return 0;
}
