#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define REQUEST_FIFO "client_to_server"
#define RESPONSE_FIFO "server_to_client"

int main() {

    char message[256];
    char response[256];

    int request_fd = open(REQUEST_FIFO, O_WRONLY);

    if (request_fd == -1) {
        perror("open request FIFO");
        exit(EXIT_FAILURE);
    }

    int response_fd = open(RESPONSE_FIFO, O_RDONLY);

    if (response_fd == -1) {
        perror("open response FIFO");
        close(request_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected to server.\n");

    while (1) {

        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        // Remove newline
        message[strcspn(message, "\n")] = '\0';

        write(request_fd, message, strlen(message));

        memset(response, 0, sizeof(response));

        ssize_t bytes_read =
            read(response_fd, response, sizeof(response) - 1);

        if (bytes_read > 0) {

            response[bytes_read] = '\0';

            printf("Server response: %s\n", response);
        }

        if (strcmp(message, "exit") == 0)
            break;
    }

    close(request_fd);
    close(response_fd);

    return 0;
}
