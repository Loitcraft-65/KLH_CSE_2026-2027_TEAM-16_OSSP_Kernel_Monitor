#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);   // create a pipe

    if (fork() == 0) {
        // Child 1: run "ls -l"
        dup2(fd[1], STDOUT_FILENO); // redirect output to pipe
        close(fd[0]); // close unused read end
        close(fd[1]);
        execlp("ls", "ls", "-l", NULL);
        perror("exec ls failed");
        exit(1);
    }

    if (fork() == 0) {
        // Child 2: run "grep .c"
        dup2(fd[0], STDIN_FILENO); // redirect input from pipe
        close(fd[1]); // close unused write end
        close(fd[0]);
        execlp("grep", "grep", ".c", NULL);
        perror("exec grep failed");
        exit(1);
    }

    // Parent process: close pipe and wait
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);

    return 0;
}
