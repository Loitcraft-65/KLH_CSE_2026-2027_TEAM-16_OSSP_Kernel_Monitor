#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char name[100];
    char command[200];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);

    name[strcspn(name, "\n")] = '\0';

    snprintf(command, sizeof(command),
             "echo \"Hello %s, your home directory is $HOME\"",
             name);

    printf("\nCommand using double quotes:\n");
    printf("%s\n", command);

    printf("\nOutput:\n");
    system(command);

    return 0;
}
