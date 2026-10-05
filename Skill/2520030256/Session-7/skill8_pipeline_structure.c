#include <stdio.h>
#include <string.h>

#define MAX_COMMANDS 10
#define MAX_LENGTH 100

int main() {
    char input[500];
    char commands[MAX_COMMANDS][MAX_LENGTH];
    int count = 0;

    printf("Enter pipeline: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = '\0';

    char *token = strtok(input, "|");

    while (token != NULL && count < MAX_COMMANDS) {
        while (*token == ' ')
            token++;

        strcpy(commands[count], token);
        count++;

        token = strtok(NULL, "|");
    }

    printf("\nPipeline Structure:\n");

    for (int i = 0; i < count; i++)
        printf("Command %d: %s\n", i + 1, commands[i]);

    printf("Total commands: %d\n", count);

    return 0;
}
