#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    char input[BUFFER_SIZE];

    printf("Welcome to MyShell!\n");

    while (1) {
        // 1. Display prompt
        printf("myshell> ");
        fflush(stdout);

        // 2. Read user input
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // 3. Remove newline from Enter key
        input[strcspn(input, "\n")] = '\0';

        // 4. Handle empty input
        if (strlen(input) == 0) {
            continue;
        }

        // 5. Handle exit condition
        if (strcmp(input, "exit") == 0) {
            printf("Exiting MyShell...\n");
            break;
        }

        // 6. Process command
        printf("You entered: %s\n", input);
    }

    return 0;
}
