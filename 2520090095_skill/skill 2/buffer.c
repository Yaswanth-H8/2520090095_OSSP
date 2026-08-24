#include <stdio.h>

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    int index = 0;
    int ch;

    printf("Type a command: ");

    while (1) {
        ch = getchar();

        // Handle Enter key
        if (ch == '\n') {
            buffer[index] = '\0';
            break;
        }

        // Handle Backspace
        if (ch == '\b' || ch == 127) {
            if (index > 0) {
                index--;

                // Move cursor back and erase character
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        // Store character in buffer
        if (index < BUFFER_SIZE - 1) {
            buffer[index] = ch;
            index++;

            // Display typed character
            putchar(ch);
            fflush(stdout);
        }
    }

    printf("\nYou entered: %s\n", buffer);

    return 0;
}
