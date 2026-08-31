#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define INITIAL_BUFFER_SIZE 64
#define HISTORY_INITIAL_SIZE 10

/* ---------- Terminal Functions ---------- */

void enableRawMode(struct termios *original)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, original);
    raw = *original;

    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios *original)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, original);
}

/* ---------- Dynamic Buffer ---------- */

char *resizeBuffer(char *buffer, size_t *size)
{
    *size *= 2;

    char *temp = realloc(buffer, *size);

    if (temp == NULL)
    {
        free(buffer);
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    return temp;
}

/* ---------- History ---------- */

typedef struct
{
    char **commands;
    size_t count;
    size_t capacity;
} History;

void initHistory(History *history)
{
    history->commands = malloc(
        HISTORY_INITIAL_SIZE * sizeof(char *)
    );

    if (history->commands == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    history->count = 0;
    history->capacity = HISTORY_INITIAL_SIZE;
}

void addHistory(History *history, const char *command)
{
    if (command[0] == '\0')
        return;

    if (history->count == history->capacity)
    {
        history->capacity *= 2;

        char **temp = realloc(
            history->commands,
            history->capacity * sizeof(char *)
        );

        if (temp == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        history->commands = temp;
    }

    history->commands[history->count] = malloc(
        strlen(command) + 1
    );

    if (history->commands[history->count] == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(history->commands[history->count], command);
    history->count++;
}

void freeHistory(History *history)
{
    for (size_t i = 0; i < history->count; i++)
        free(history->commands[i]);

    free(history->commands);
}

/* ---------- Display Input ---------- */

void clearLine(size_t length)
{
    printf("\r");

    for (size_t i = 0; i < length; i++)
        printf(" ");

    printf("\r");
}

void printPrompt(const char *buffer)
{
    printf("\r\033[K");
    printf("shell> %s", buffer);
    fflush(stdout);
}

/* ---------- Read Command ---------- */

char *readCommand(History *history)
{
    size_t bufferSize = INITIAL_BUFFER_SIZE;
    char *buffer = malloc(bufferSize);

    if (buffer == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t length = 0;
    size_t cursor = 0;

    buffer[0] = '\0';

    /* Start after the newest command */
    size_t historyIndex = history->count;

    while (1)
    {
        char c;

        if (read(STDIN_FILENO, &c, 1) != 1)
            continue;

        /* Enter key */
        if (c == '\n' || c == '\r')
        {
            buffer[length] = '\0';
            printf("\n");
            return buffer;
        }

        /* Backspace */
        if (c == 127 || c == '\b')
        {
            if (cursor > 0)
            {
                memmove(
                    &buffer[cursor - 1],
                    &buffer[cursor],
                    length - cursor + 1
                );

                cursor--;
                length--;

                printPrompt(buffer);

                printf("\033[%zuC", cursor + 7);
                fflush(stdout);
            }

            continue;
        }

        /* Escape sequence */
        if (c == 27)
        {
            char seq[2];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
                continue;

            if (seq[0] != '[')
                continue;

            if (read(STDIN_FILENO, &seq[1], 1) != 1)
                continue;

            /* Up arrow */
            if (seq[1] == 'A')
            {
                if (history->count > 0 && historyIndex > 0)
                {
                    historyIndex--;

                    clearLine(length);

                    strcpy(buffer,
                           history->commands[historyIndex]);

                    length = strlen(buffer);
                    cursor = length;

                    printPrompt(buffer);
                }
            }

            /* Down arrow */
            else if (seq[1] == 'B')
            {
                if (historyIndex < history->count)
                {
                    historyIndex++;

                    clearLine(length);

                    if (historyIndex == history->count)
                    {
                        buffer[0] = '\0';
                        length = 0;
                    }
                    else
                    {
                        strcpy(buffer,
                               history->commands[historyIndex]);

                        length = strlen(buffer);
                    }

                    cursor = length;

                    printPrompt(buffer);
                }
            }

            /* Right arrow */
            else if (seq[1] == 'C')
            {
                if (cursor < length)
                {
                    cursor++;
                    printf("\033[C");
                    fflush(stdout);
                }
            }

            /* Left arrow */
            else if (seq[1] == 'D')
            {
                if (cursor > 0)
                {
                    cursor--;
                    printf("\033[D");
                    fflush(stdout);
                }
            }

            continue;
        }

        /* Normal character */
        if (c >= 32 && c <= 126)
        {
            if (length + 1 >= bufferSize)
                buffer = resizeBuffer(buffer, &bufferSize);

            memmove(
                &buffer[cursor + 1],
                &buffer[cursor],
                length - cursor + 1
            );

            buffer[cursor] = c;

            length++;
            cursor++;

            printPrompt(buffer);

            /* Move cursor to correct position */
            if (cursor < length)
                printf("\033[%zuD", length - cursor);

            fflush(stdout);
        }
    }
}

/* ---------- Linked List ---------- */

typedef struct Node
{
    char *data;
    struct Node *next;
} Node;

Node *createNode(const char *data)
{
    Node *newNode = malloc(sizeof(Node));

    if (newNode == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newNode->data = malloc(strlen(data) + 1);

    if (newNode->data == NULL)
    {
        free(newNode);
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->data, data);

    newNode->next = NULL;

    return newNode;
}

void appendNode(Node **head, const char *data)
{
    Node *newNode = createNode(data);

    if (*head == NULL)
    {
        *head = newNode;
        return;
    }

    Node *temp = *head;

    while (temp->next != NULL)
        temp = temp->next;

    temp->next = newNode;
}

void freeList(Node *head)
{
    Node *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;

        free(temp->data);
        free(temp);
    }
}

/* ---------- Main ---------- */

int main()
{
    struct termios originalTerminal;
    History history;

    initHistory(&history);

    enableRawMode(&originalTerminal);

    printf("Simple Shell with Command History\n");
    printf("Use UP/DOWN arrows to recall commands.\n");
    printf("Use LEFT/RIGHT arrows to move cursor.\n");
    printf("Press Ctrl+D or type exit to quit.\n\n");

    while (1)
    {
        printf("shell> ");
        fflush(stdout);

        char *command = readCommand(&history);

        if (strcmp(command, "exit") == 0)
        {
            free(command);
            break;
        }

        if (command[0] != '\0')
        {
            addHistory(&history, command);

            /*
             * Demonstrating linked-list management
             */
            Node *list = NULL;

            appendNode(&list, command);

            freeList(list);
        }

        free(command);
    }

    disableRawMode(&originalTerminal);

    freeHistory(&history);

    printf("Memory released successfully.\n");

    return 0;
}
