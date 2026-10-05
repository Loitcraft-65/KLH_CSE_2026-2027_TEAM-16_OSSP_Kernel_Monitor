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
        printf("Foreground job started. PID: %d\n", getpid());
        sleep(3);
        printf("Foreground job completed.\n");
        exit(0);
    }

    printf("Job created with PID: %d\n", pid);
    printf("Transferring job to foreground...\n");

    waitpid(pid, NULL, 0);

    printf("Foreground job finished.\n");

    return 0;
}
