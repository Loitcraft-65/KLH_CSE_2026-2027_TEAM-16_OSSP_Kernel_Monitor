#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child PID: %d\n", getpid());
        printf("Child PGID: %d\n", getpgrp());

        for (int i = 1; i <= 5; i++) {
            printf("Child running: %d\n", i);
            sleep(1);
        }

        exit(0);
    }

    printf("Parent PID: %d\n", getpid());
    printf("Parent PGID: %d\n", getpgrp());

    printf("Child PID: %d\n", pid);

    waitpid(pid, NULL, 0);

    printf("Child process completed.\n");

    return 0;
}
