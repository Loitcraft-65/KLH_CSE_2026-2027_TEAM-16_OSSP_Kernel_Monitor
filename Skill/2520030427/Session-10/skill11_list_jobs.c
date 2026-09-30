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
        sleep(5);
        exit(0);
    }

    printf("Active Jobs:\n");
    printf("Job ID\tPID\tStatus\n");
    printf("[1]\t%d\tRunning\n", pid);

    sleep(2);

    if (waitpid(pid, NULL, WNOHANG) == 0)
        printf("[1]\t%d\tStill Running\n", pid);

    waitpid(pid, NULL, 0);

    printf("[1]\t%d\tCompleted\n", pid);

    return 0;
}
