#include <stdio.h>
#include <string.h>

int main() {
    char command[100];

    printf("Enter command: ");
    fgets(command, sizeof(command), stdin);

    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "pwd") == 0) {
        printf("Integration test: pwd command accepted.\n");
    } else if (strcmp(command, "ls") == 0) {
        printf("Integration test: ls command accepted.\n");
    } else if (strcmp(command, "exit") == 0) {
        printf("Integration test: exit command accepted.\n");
    } else {
        printf("Integration test: unknown command.\n");
    }

    return 0;
}
