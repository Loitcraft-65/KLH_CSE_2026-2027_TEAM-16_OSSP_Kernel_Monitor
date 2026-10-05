#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("output.txt");
        return 1;
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);

    printf("This output is redirected to a file.\n");
    printf("OSSP Week 9 Output Redirection\n");

    return 0;
}
