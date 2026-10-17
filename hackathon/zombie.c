#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child (PID %d) exiting\n", getpid());
        exit(0);               // child dies immediately
    } else {
        printf("Parent (PID %d) sleeping, NOT calling wait()\n", getpid());
        sleep(60);             // parent never reaps -> zombie
    }
    return 0;
}
