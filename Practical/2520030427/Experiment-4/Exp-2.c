#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
    int fd;
    char buffer[100];
    ssize_t bytes_read;

    // Open the file
    fd = open("input.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    // Read data from the file
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1)
    {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    // Add null character at the end
    buffer[bytes_read] = '\0';

    // Display the data
    printf("Contents of the file:\n");
    printf("%s\n", buffer);

    // Close the file
    close(fd);

    return 0;
}


