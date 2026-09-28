#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signal) {

    if (signal == SIGINT) {
        printf("\nSIGINT received! Ctrl+C detected.\n");
    }

    else if (signal == SIGTERM) {
        printf("\nSIGTERM received! Termination requested.\n");
    }

    else if (signal == SIGUSR1) {
        printf("\nSIGUSR1 received! User-defined event occurred.\n");
    }
}

int main() {

    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);

    printf("Process ID: %d\n", getpid());
    printf("Signal handling program started.\n");

    printf("Press Ctrl+C for SIGINT.\n");
    printf("Use 'kill -SIGTERM %d' for SIGTERM.\n", getpid());
    printf("Use 'kill -SIGUSR1 %d' for SIGUSR1.\n", getpid());

    while (1) {

        printf("Program is running...\n");

        sleep(3);
    }

    return 0;
}
