// producer_consumer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

#define DATA_SIZE 1000000

int main() {
    int pipefd[2];
    pid_t pid;

    char *data = malloc(DATA_SIZE);
    char buffer[4096];

    // Generate sample data
    memset(data, 'A', DATA_SIZE);

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    struct timeval start, end;

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent - Producer
        close(pipefd[0]);

        gettimeofday(&start, NULL);

        ssize_t total_written = 0;

        while (total_written < DATA_SIZE) {
            ssize_t n = write(pipefd[1],
                              data + total_written,
                              DATA_SIZE - total_written);

            if (n <= 0) {
                perror("write");
                break;
            }

            total_written += n;
        }

        close(pipefd[1]);

        wait(NULL);

        gettimeofday(&end, NULL);

        double time_taken =
            (end.tv_sec - start.tv_sec) +
            (end.tv_usec - start.tv_usec) / 1000000.0;

        double throughput =
            (total_written / (1024.0 * 1024.0)) / time_taken;

        printf("\n--- Communication Efficiency ---\n");
        printf("Data transferred : %ld bytes\n", total_written);
        printf("Time taken       : %.6f seconds\n", time_taken);
        printf("Throughput       : %.2f MB/s\n", throughput);

        free(data);
    }
    else {
        // Child - Consumer
        close(pipefd[1]);

        ssize_t total_read = 0;
        ssize_t n;

        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            total_read += n;
        }

        close(pipefd[0]);

        printf("Child consumed %ld bytes\n", total_read);

        exit(EXIT_SUCCESS);
    }

    return 0;
}
