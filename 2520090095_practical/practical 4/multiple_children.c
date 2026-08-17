#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t child1, child2, child3;
    int status;

    printf("Parent PID: %d\n", getpid());

    // Create Child 1
    child1 = fork();

    if (child1 == 0) {
        printf("Child 1: PID = %d\n", getpid());
        sleep(3);
        printf("Child 1 completed\n");
        exit(10);
    }

    // Create Child 2
    child2 = fork();

    if (child2 == 0) {
        printf("Child 2: PID = %d\n", getpid());
        sleep(1);
        printf("Child 2 completed\n");
        exit(20);
    }

    // Create Child 3
    child3 = fork();

    if (child3 == 0) {
        printf("Child 3: PID = %d\n", getpid());
        sleep(2);
        printf("Child 3 completed\n");
        exit(30);
    }

    // Parent process
    printf("\nParent using wait():\n");

    pid_t finished = wait(&status);

    printf("wait() collected Child PID: %d\n", finished);

    if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    }

    printf("\nParent using waitpid():\n");

    waitpid(child1, &status, 0);

    printf("waitpid() collected Child 1: PID = %d\n", child1);

    if (WIFEXITED(status)) {
        printf("Child 1 exit status: %d\n", WEXITSTATUS(status));
    }

    // Collect remaining children
    waitpid(child2, NULL, 0);
    waitpid(child3, NULL, 0);

    printf("\nAll children have completed.\n");

    return 0;
}
