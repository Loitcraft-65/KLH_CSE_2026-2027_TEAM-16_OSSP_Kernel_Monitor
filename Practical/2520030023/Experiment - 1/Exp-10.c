#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
int main() {
	char command[100];
	printf("Enter a Linux command: ");
	scanf("%99s", command);
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		return 1;
	}
	else if (pid == 0) {
		printf("\nChild Process\n");
		printf("Child PID  : %d\n", getpid());
        	printf("Parent PID : %d\n", getppid());
        	printf("Executing command: %s\n\n", command);
        	execlp(command, command, NULL);

	        perror("Execution failed");
        	exit(1);
    	}
    	else {
        	printf("\nParent Process\n");
        	printf("Parent PID : %d\n", getpid());
        	printf("Child PID  : %d\n", pid);
        	wait(NULL);
        	printf("\nChild process completed.\n");
    	}
    	return 0;
}
