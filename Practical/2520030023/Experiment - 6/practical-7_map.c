#include <stdio.h>
#include <unistd.h>

int main() {
    char filename[100];
    char line[500];
    FILE *file;

    printf("Process ID: %d\n", getpid());

    snprintf(filename, sizeof(filename),
             "/proc/%d/maps", getpid());

    file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    printf("\nVirtual Memory Mappings\n");
    printf("=======================\n");

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);

    return 0;
}
