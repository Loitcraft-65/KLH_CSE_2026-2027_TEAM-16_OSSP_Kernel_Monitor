#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;

    printf("Creating Child Process...\n\n");

    pid = fork();

    if (pid < 0)
    {
        printf("Fork Failed!\n");
        return 1;
    }

    else if (pid == 0)
    {
        printf("CHILD PROCESS\n");
        printf("Child PID          : %d\n", getpid());
        printf("Parent PID (PPID)  : %d\n", getppid());
        printf("Process State      : Running\n");

        sleep(5);

        printf("Process State      : Terminated\n");
    }

    else
    {
        printf("PARENT PROCESS\n");
        printf("Parent PID\t: %d\n", getpid());
        printf("Child PID\t: %d\n", pid);
        printf("Process State\t: Waiting\n");

        wait(NULL);

        printf("Process State\t: Running\n");
        printf("Child Process Completed.\n");
    }

    return 0;
}

/* OUTPUT:-
PARENT PROCESS
Parent PID      : 750
Child PID       : 751
Process State   : Waiting
CHILD PROCESS
Child PID          : 751
Parent PID (PPID)  : 750
Process State      : Running
Process State      : Terminated
Process State   : Running
Child Process Completed.
*/

