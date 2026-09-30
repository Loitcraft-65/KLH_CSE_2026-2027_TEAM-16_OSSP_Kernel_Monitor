#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process started. PID: %d\n", getpid());

        for (int i = 1; i <= 5; i++) {
            printf("Child running: %d\n", i);
            sleep(1);
        }

        exit(0);
    }

    sleep(1);

    printf("Stopping job %d...\n", pid);
    kill(pid, SIGSTOP);

    sleep(1);

    int status;

    waitpid(pid, &status, WUNTRACED);

    if (WIFSTOPPED(status))
        printf("Job is stopped.\n");

    printf("Sending SIGCONT...\n");
    kill(pid, SIGCONT);

    waitpid(pid, &status, 0);

    printf("Job resumed and completed.\n");

    return 0;
}
