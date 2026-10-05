#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        int fd = open("pipeline_output.txt",
                      O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0) {
            perror("pipeline_output.txt");
            exit(1);
        }

        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execlp("ls", "ls", NULL);

        perror("ls");
        exit(1);
    }

    close(pipefd[1]);

    int output = open("pipeline_output.txt",
                      O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (output < 0) {
        perror("pipeline_output.txt");
        return 1;
    }

    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);

    wait(NULL);

    dup2(output, STDOUT_FILENO);
    close(output);

    char buffer[1024];
    int n;

    while ((n = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
        write(STDOUT_FILENO, buffer, n);

    return 0;
}
