#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int sig) {
    printf("\nSIGINT received. Program is still running.\n");
}

int main() {
    signal(SIGINT, sigint_handler);

    printf("PID: %d\n", getpid());
    printf("Press Ctrl+C to send SIGINT.\n");

    while (1) {
        printf("Program running...\n");
        sleep(2);
    }

    return 0;
}
