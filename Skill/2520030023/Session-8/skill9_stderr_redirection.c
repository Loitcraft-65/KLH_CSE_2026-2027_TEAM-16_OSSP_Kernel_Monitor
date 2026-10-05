#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("error.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("error.txt");
        return 1;
    }

    dup2(fd, STDERR_FILENO);
    close(fd);

    fprintf(stderr, "This is an error message.\n");
    fprintf(stderr, "OSSP Week 9 stderr redirection.\n");

    return 0;
}
