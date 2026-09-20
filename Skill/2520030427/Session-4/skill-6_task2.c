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
        printf("Executing ls command...\n");

        execlp("ls", "ls", "-l", NULL);

        perror("execlp");
        exit(1);
    } else {
        printf("Parent is waiting for child...\n");

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child exited with status: %d\n",
                   WEXITSTATUS(status));
        }
    }

    return 0;
}
