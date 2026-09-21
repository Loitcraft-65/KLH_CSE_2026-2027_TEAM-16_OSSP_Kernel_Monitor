#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char command[100];
    char *path;
    char *path_copy;
    char *directory;
    char fullpath[500];

    printf("Enter command to search: ");
    scanf("%99s", command);

    path = getenv("PATH");

    if (path == NULL) {
        printf("PATH variable not found.\n");
        return 1;
    }

    path_copy = strdup(path);

    if (path_copy == NULL) {
        perror("strdup");
        return 1;
    }

    directory = strtok(path_copy, ":");

    while (directory != NULL) {
        snprintf(fullpath, sizeof(fullpath),
                 "%s/%s", directory, command);

        if (access(fullpath, X_OK) == 0) {
            printf("Executable found:\n");
            printf("%s\n", fullpath);

            free(path_copy);
            return 0;
        }

        directory = strtok(NULL, ":");
    }

    printf("Command '%s' not found in PATH.\n", command);

    free(path_copy);

    return 0;
}
