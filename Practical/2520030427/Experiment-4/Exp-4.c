#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int source_fd, destination_fd;
    char buffer[1024];
    int bytes_read;
    char source[100], destination[100];

    // Get file names
    printf("Enter source file name: ");
    scanf("%s", source);
    printf("Enter destination file name: ");
    scanf("%s", destination);

    // Open source file in read-only mode
    source_fd = open(source, O_RDONLY);
    if (source_fd == -1) {
        perror("Error opening source file");
        return 1;
    }

    // Open/create destination file in write mode
    destination_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_fd == -1) {
        perror("Error opening/creating destination file");
        close(source_fd);
        return 1;
    }

    // Read from source and write to destination
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        if (write(destination_fd, buffer, bytes_read) == -1) {
            perror("Error writing to destination file");
            close(source_fd);
            close(destination_fd);
            return 1;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading source file");
    }

    // Close both files
    close(source_fd);
    close(destination_fd);

    printf("File copied successfully!\n");
    return 0;
}
