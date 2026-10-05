#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main()
{
    int fd;
    struct stat st;
    char *data;

    fd = open("mmapfile.txt", O_RDONLY);

    if (fd < 0)
    {
        perror("Error opening file");
        return 1;
    }

    if (fstat(fd, &st) < 0)
    {
        perror("fstat failed");
        close(fd);
        return 1;
    }

    if (st.st_size == 0)
    {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (data == MAP_FAILED)
    {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    printf("File contents using mmap():\n");
    write(STDOUT_FILENO, data, st.st_size);

    munmap(data, st.st_size);
    close(fd);

    return 0;
}
