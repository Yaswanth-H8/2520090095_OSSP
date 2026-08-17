#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        printf("Child PID: %d\n", getpid());
        printf("Child terminating...\n");

        exit(0);
    }
    else {
        printf("Parent PID: %d\n", getpid());
        printf("Child PID: %d\n", pid);

        printf("Parent is NOT calling wait().\n");
        printf("Parent sleeping for 20 seconds...\n");

        sleep(20);

        printf("Parent terminating...\n");
    }

    return 0;
}
