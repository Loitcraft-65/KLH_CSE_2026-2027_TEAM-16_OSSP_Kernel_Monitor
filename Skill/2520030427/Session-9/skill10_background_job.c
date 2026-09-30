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
        printf("Background job started. PID: %d\n", getpid());
        sleep(5);
        printf("Background job completed.\n");
        exit(0);
    }

    printf("Parent continues immediately.\n");
    printf("Background Job PID: %d\n", pid);

    sleep(1);

    if (waitpid(pid, NULL, WNOHANG) == 0)
        printf("Job is still running.\n");

    waitpid(pid, NULL, 0);

    printf("Job finished.\n");

    return 0;
}
