#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define REQUEST_FIFO "client_to_server"
#define RESPONSE_FIFO "server_to_client"

int main() {
    char buffer[256];

    // Create named pipes
    mkfifo(REQUEST_FIFO, 0666);
    mkfifo(RESPONSE_FIFO, 0666);

    printf("Server started...\n");
    printf("Waiting for client messages...\n");

    int request_fd = open(REQUEST_FIFO, O_RDONLY);

    if (request_fd == -1) {
        perror("open request FIFO");
        exit(EXIT_FAILURE);
    }

    int response_fd = open(RESPONSE_FIFO, O_WRONLY);

    if (response_fd == -1) {
        perror("open response FIFO");
        close(request_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {

        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes_read = read(request_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {

            buffer[bytes_read] = '\0';

            printf("Client message: %s\n", buffer);

            // Exit command
            if (strncmp(buffer, "exit", 4) == 0) {
                char response[] = "Server shutting down...";
                write(response_fd, response, strlen(response));

                break;
            }

            // Process message
            char response[256];

            snprintf(response, sizeof(response),
                     "Server processed: %s", buffer);

            write(response_fd, response, strlen(response));

            printf("Response sent to client.\n");
        }
    }

    close(request_fd);
    close(response_fd);

    unlink(REQUEST_FIFO);
    unlink(RESPONSE_FIFO);

    printf("Server terminated.\n");

    return 0;
}
