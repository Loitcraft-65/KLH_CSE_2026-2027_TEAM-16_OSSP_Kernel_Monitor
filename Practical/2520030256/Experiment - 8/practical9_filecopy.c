#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int src, dest;
    char buffer[1024];
    ssize_t bytes;
    off_t position;

    src = open("source.txt", O_RDONLY);

    if (src < 0)
    {
        perror("Error opening source file");
        return 1;
    }

    dest = open("destination.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (dest < 0)
    {
        perror("Error opening destination file");
        close(src);
        return 1;
    }

    while ((bytes = read(src, buffer, sizeof(buffer))) > 0)
    {
        if (write(dest, buffer, bytes) != bytes)
        {
            perror("Error writing file");
            close(src);
            close(dest);
            return 1;
        }
    }

    position = lseek(src, 0, SEEK_CUR);

    printf("File copied successfully.\n");
    printf("Current file position: %ld bytes\n", (long)position);

    close(src);
    close(dest);

    return 0;
}
