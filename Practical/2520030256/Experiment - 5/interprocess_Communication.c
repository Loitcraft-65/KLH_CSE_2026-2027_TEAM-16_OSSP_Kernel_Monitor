
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int fd[2];
    pid_t pid;
    char buffer[100];

    // Create pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork process
    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Child process (Consumer)
        close(fd[1]); // close write end
        read(fd[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
        close(fd[0]);
    } else {
        // Parent process (Producer)
        close(fd[0]); // close read end
        char msg[] = "Hello from parent!";
        write(fd[1], msg, sizeof(msg));
        close(fd[1]);
    }

    return 0;
}


