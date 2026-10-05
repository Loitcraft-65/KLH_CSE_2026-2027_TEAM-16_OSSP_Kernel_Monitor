#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int fd;

    fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
    {
        perror("Error opening output file");
        return 1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0)
    {
        perror("dup2 failed");
        close(fd);
        return 1;
    }

    close(fd);

    printf("This output is redirected to output.txt\n");
    printf("dup2() successfully redirected standard output.\n");

    return 0;
}
