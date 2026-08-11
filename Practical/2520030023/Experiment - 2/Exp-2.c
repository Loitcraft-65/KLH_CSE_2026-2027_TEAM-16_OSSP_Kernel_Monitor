#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int i;
    printf("Parent Process PID : %d\n\n", getpid());
    for(i = 1; i <= 3; i++)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            printf("Child %d Created\n", i);
            printf("PID  : %d\n", getpid());
            printf("PPID : %d\n", getppid());

            sleep(i + 1);

            printf("Child %d Finished\n\n", i);
            exit(0);
        }
    }
    printf("Parent waiting for all child processes...\n\n");
    for(i = 1; i <= 3; i++)
    {
        wait(NULL);
        printf("One child process completed.\n");
    }
    printf("\nAll Child Processes Completed.\n");
    return 0;
}

/* OUTPUT:-
Parent Process PID : 816

Child 1 Created
PID  : 817
PPID : 816
Child 2 Created
PID  : 818
PPID : 816
Child 3 Created
PID  : 819
PPID : 816
Parent waiting for all child processes...

Child 1 Finished

One child process completed.
Child 2 Finished

One child process completed.
Child 3 Finished

One child process completed.

All Child Processes Completed.
*/

