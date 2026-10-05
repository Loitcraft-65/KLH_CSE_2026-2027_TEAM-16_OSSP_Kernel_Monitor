#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("append.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd < 0) {
        perror("append.txt");
        return 1;
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);

    printf("New line added using append redirection.\n");

    return 0;
}
