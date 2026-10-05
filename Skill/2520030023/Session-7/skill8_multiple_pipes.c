#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t p1 = fork();

    if (p1 == 0) {
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        execlp("ls", "ls", NULL);
        perror("ls");
        exit(1);
    }

    pid_t p2 = fork();

    if (p2 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe2[1]);

        execlp("grep", "grep", ".c", NULL);
        perror("grep");
        exit(1);
    }

    pid_t p3 = fork();

    if (p3 == 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);

        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        execlp("wc", "wc", "-l", NULL);
        perror("wc");
        exit(1);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    waitpid(p3, NULL, 0);

    return 0;
}
