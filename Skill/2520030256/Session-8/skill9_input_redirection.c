#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd = open("input.txt", O_RDONLY);

    if (fd < 0) {
        perror("input.txt");
        return 1;
    }

    dup2(fd, STDIN_FILENO);
    close(fd);

    char buffer[100];
    int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read");
        return 1;
    }

    buffer[n] = '\0';

    printf("Input received:\n%s", buffer);

    return 0;
}
