#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    printf("\nSignal %d received.\n", sig);
}

int main() {
    signal(SIGUSR1, handler);

    printf("Process ID: %d\n", getpid());
    printf("Waiting for SIGUSR1...\n");

    for (int i = 1; i <= 10; i++) {
        printf("Running: %d\n", i);
        sleep(1);
    }

    return 0;
}
