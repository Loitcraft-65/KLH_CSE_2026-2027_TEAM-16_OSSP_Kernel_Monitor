#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void signal_handler(int sig) {
    printf("Signal %d received by process %d\n", sig, getpid());
}

int main() {
    signal(SIGINT, signal_handler);

    printf("Process ID: %d\n", getpid());
    printf("Waiting for SIGINT...\n");
    printf("Press Ctrl+C to send SIGINT.\n");

    for (int i = 1; i <= 10; i++) {
        printf("Running: %d\n", i);
        sleep(1);
    }

    printf("Program completed.\n");

    return 0;
}
