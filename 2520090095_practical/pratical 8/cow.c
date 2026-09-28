#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 1000000

int main() {

    // Allocate approximately 4 MB
    int *data = malloc(SIZE * sizeof(int));

    if (data == NULL) {
        perror("malloc");
        return 1;
    }

    // Initialize memory
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
    }

    printf("Parent PID: %d\n", getpid());
    printf("Memory initialized.\n");

    printf("Press Enter to perform fork()...\n");
    getchar();

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        free(data);
        return 1;
    }

    if (pid == 0) {

        // Child process
        printf("\nChild PID: %d\n", getpid());

        printf("Child created after fork().\n");
        printf("Child is now modifying memory...\n");

        // Modify every page
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] + 1;
        }

        printf("Child memory modification completed.\n");

        printf("Press Enter to terminate child...\n");
        getchar();

        free(data);

        exit(0);

    } else {

        // Parent process
        printf("\nParent PID: %d\n", getpid());
        printf("Child PID: %d\n", pid);

        printf("Parent memory has NOT been modified.\n");

        printf("Press Enter to terminate parent...\n");
        getchar();

        wait(NULL);

        free(data);
    }

    return 0;
}
