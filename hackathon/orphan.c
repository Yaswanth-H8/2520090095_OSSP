#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid > 0) {
        printf("Parent (PID %d) exiting\n", getpid());
        exit(0);               // parent dies first
    } else {
        sleep(2);
        printf("Child (PID %d) new parent PID: %d\n", getpid(), getppid());
        sleep(60);             // child keeps running -> orphan
    }
    return 0;
}
