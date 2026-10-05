#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigtstp_handler(int sig) {
    printf("\nSIGTSTP received. Stop request handled.\n");
}

int main() {
    signal(SIGTSTP, sigtstp_handler);

    printf("PID: %d\n", getpid());
    printf("Press Ctrl+Z to send SIGTSTP.\n");

    while (1) {
        printf("Process running...\n");
        sleep(2);
    }

    return 0;
}
