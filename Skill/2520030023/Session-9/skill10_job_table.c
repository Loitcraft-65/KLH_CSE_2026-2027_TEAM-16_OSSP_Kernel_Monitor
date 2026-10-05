#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_JOBS 10

struct Job {
    int id;
    pid_t pid;
    char state[20];
};

int main() {
    struct Job jobs[MAX_JOBS];
    int count = 0;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        sleep(3);
        exit(0);
    }

    jobs[count].id = count + 1;
    jobs[count].pid = pid;
    snprintf(jobs[count].state, sizeof(jobs[count].state), "Running");
    count++;

    printf("Job Table:\n");
    printf("ID\tPID\tState\n");

    for (int i = 0; i < count; i++)
        printf("%d\t%d\t%s\n",
               jobs[i].id,
               jobs[i].pid,
               jobs[i].state);

    sleep(1);

    if (waitpid(pid, NULL, WNOHANG) == 0) {
        printf("\nJob %d is still running.\n", jobs[0].id);
    }

    waitpid(pid, NULL, 0);

    snprintf(jobs[0].state,
             sizeof(jobs[0].state),
             "Completed");

    printf("\nUpdated Job Table:\n");
    printf("ID\tPID\tState\n");

    for (int i = 0; i < count; i++)
        printf("%d\t%d\t%s\n",
               jobs[i].id,
               jobs[i].pid,
               jobs[i].state);

    return 0;
}
