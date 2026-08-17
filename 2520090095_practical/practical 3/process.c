#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void display_process_info(const char *stage) {
    printf("\n--- %s ---\n", stage);
    printf("PID  : %d\n", getpid());
    printf("PPID : %d\n", getppid());
}

int main() {
    pid_t pid;

    display_process_info("Parent Before fork()");

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process
        display_process_info("Child Created - Running");

        printf("Child is now performing some work...\n");

        // Sleep makes the child observable in ps/top
        sleep(10);

        display_process_info("Child After Waiting");

        printf("Child process is terminating.\n");
        exit(0);
    }
    else {
        // Parent process
        display_process_info("Parent After fork()");

        printf("Child PID: %d\n", pid);
        printf("Parent is waiting for the child...\n");

        // Parent enters waiting state
        wait(NULL);

        display_process_info("Parent After Child Termination");

        printf("Child has terminated.\n");
    }

    return 0;
}
