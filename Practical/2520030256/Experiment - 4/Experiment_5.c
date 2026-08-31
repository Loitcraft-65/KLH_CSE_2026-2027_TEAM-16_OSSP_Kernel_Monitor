#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;

    // Open the file
    fd = open("input.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    printf("File opened successfully.\n");
    printf("File descriptor: %d\n", fd);

    // Close the file
    if (close(fd) == -1)
    {
        perror("Error closing file");
        return 1;
    }

    printf("File closed successfully.\n");

    return 0;
}
