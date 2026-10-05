#include <stdio.h>
#include <string.h>

#define MAX_HISTORY 10
#define MAX_COMMAND 100

int main() {
    char history[MAX_HISTORY][MAX_COMMAND];
    int count = 0;
    char command[MAX_COMMAND];

    while (1) {
        printf("shell> ");
        fgets(command, MAX_COMMAND, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0)
            break;

        if (strlen(command) == 0)
            continue;

        if (count < MAX_HISTORY) {
            strcpy(history[count], command);
            count++;
        } else {
            for (int i = 0; i < MAX_HISTORY - 1; i++)
                strcpy(history[i], history[i + 1]);

            strcpy(history[MAX_HISTORY - 1], command);
        }

        if (strcmp(command, "history") == 0) {
            printf("\nCommand History:\n");

            for (int i = 0; i < count; i++)
                printf("%d  %s\n", i + 1, history[i]);
        }
    }

    return 0;
}
