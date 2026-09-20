#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    int status;

    printf("Parent Process ID: %d\n", getpid());

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child Process ID: %d\n", getpid());
        printf("Child is running...\n");

        sleep(2);

        printf("Child completed.\n");
        exit(5);
    } else {
        printf("Parent is waiting using waitpid()...\n");

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child terminated normally.\n");
            printf("Child exit status: %d\n",
                   WEXITSTATUS(status));
        }

        printf("Parent process completed.\n");
    }

    return 0;
}
