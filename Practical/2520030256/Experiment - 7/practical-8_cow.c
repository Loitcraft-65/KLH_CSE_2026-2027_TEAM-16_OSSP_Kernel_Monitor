#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 100000

int main() {
    int *data;
    pid_t pid;

    data = malloc(SIZE * sizeof(int));

    if (data == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
    }

    printf("Parent PID: %d\n", getpid());
    printf("Initial value: %d\n", data[0]);

    pid = fork();

    if (pid < 0) {
        perror("fork");
        free(data);
        return 1;
    }

    if (pid == 0) {
        printf("\nChild PID: %d\n", getpid());

        printf("Value before modification: %d\n", data[0]);

        data[0] = 9999;

        printf("Value after modification: %d\n", data[0]);
        printf("Child has modified its private copy.\n");

        free(data);
        exit(0);
    } else {
        waitpid(pid, NULL, 0);

        printf("\nParent after child terminates:\n");
        printf("Value in parent: %d\n", data[0]);
        printf("Parent value remains unchanged.\n");

        free(data);
    }

    return 0;
}
