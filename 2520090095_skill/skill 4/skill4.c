#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 256

/* ---------- Token Types ---------- */

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_APPEND,
    TOKEN_END
} TokenType;

/* ---------- Token Structure ---------- */

typedef struct Token
{
    TokenType type;
    char *value;
    struct Token *next;
} Token;

/* ---------- Parse Tree ---------- */

typedef struct Command
{
    Token **tokens;
    int count;
    int capacity;

    char *input_file;
    char *output_file;
    int append;

    struct Command *next;
} Command;

/* ---------- Utility Functions ---------- */

char *duplicateString(const char *str)
{
    char *copy = malloc(strlen(str) + 1);

    if (copy == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, str);
    return copy;
}

const char *tokenName(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_REDIRECT_IN:
            return "REDIRECT_IN";

        case TOKEN_REDIRECT_OUT:
            return "REDIRECT_OUT";

        case TOKEN_APPEND:
            return "APPEND";

        case TOKEN_END:
            return "END";

        default:
            return "UNKNOWN";
    }
}

/* ---------- Create Token ---------- */

Token *createToken(TokenType type, const char *value)
{
    Token *token = malloc(sizeof(Token));

    if (token == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token->type = type;
    token->value = duplicateString(value);
    token->next = NULL;

    return token;
}

/* ---------- Add Token ---------- */

void addToken(Token **head, Token **tail,
              TokenType type, const char *value)
{
    Token *newToken = createToken(type, value);

    if (*head == NULL)
    {
        *head = newToken;
        *tail = newToken;
    }
    else
    {
        (*tail)->next = newToken;
        *tail = newToken;
    }
}

/* ---------- Lexer / Tokenizer ---------- */

Token *tokenize(const char *input)
{
    Token *head = NULL;
    Token *tail = NULL;

    int i = 0;

    while (input[i] != '\0')
    {
        /* Ignore whitespace */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (input[i] == '|')
        {
            addToken(&head, &tail, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            addToken(&head, &tail, TOKEN_REDIRECT_IN, "<");
            i++;
            continue;
        }

        /* Output redirection / append */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                addToken(&head, &tail, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                addToken(&head, &tail, TOKEN_REDIRECT_OUT, ">");
                i++;
            }

            continue;
        }

        /* Word token */
        char buffer[MAX_INPUT];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>')
        {
            if (j < MAX_INPUT - 1)
            {
                buffer[j++] = input[i];
            }

            i++;
        }

        buffer[j] = '\0';

        if (j > 0)
        {
            addToken(&head, &tail, TOKEN_WORD, buffer);
        }
    }

    addToken(&head, &tail, TOKEN_END, "END");

    return head;
}

/* ---------- Debug Token Output ---------- */

void printTokens(Token *head)
{
    printf("\n========== TOKEN STREAM ==========\n");

    int index = 0;

    while (head != NULL)
    {
        printf("[%d] %-15s : %s\n",
               index,
               tokenName(head->type),
               head->value);

        if (head->type == TOKEN_END)
            break;

        head = head->next;
        index++;
    }

    printf("==================================\n");
}

/* ---------- Validate Token Stream ---------- */

int validateTokens(Token *head)
{
    if (head == NULL || head->type == TOKEN_END)
    {
        printf("Empty command.\n");
        return 0;
    }

    Token *current = head;

    if (current->type == TOKEN_PIPE)
    {
        printf("Syntax Error: Command cannot start with '|'.\n");
        return 0;
    }

    while (current != NULL &&
           current->type != TOKEN_END)
    {
        /* Pipe validation */
        if (current->type == TOKEN_PIPE)
        {
            if (current->next == NULL ||
                current->next->type == TOKEN_PIPE ||
                current->next->type == TOKEN_END)
            {
                printf("Syntax Error: Invalid pipe placement.\n");
                return 0;
            }
        }

        /* Redirection validation */
        if (current->type == TOKEN_REDIRECT_IN ||
            current->type == TOKEN_REDIRECT_OUT ||
            current->type == TOKEN_APPEND)
        {
            if (current->next == NULL ||
                current->next->type != TOKEN_WORD)
            {
                printf("Syntax Error: Redirection requires a filename.\n");
                return 0;
            }
        }

        current = current->next;
    }

    return 1;
}

/* ---------- Create Command ---------- */

Command *createCommand(void)
{
    Command *cmd = malloc(sizeof(Command));

    if (cmd == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    cmd->capacity = 4;
    cmd->count = 0;

    cmd->tokens = malloc(
        cmd->capacity * sizeof(Token *)
    );

    if (cmd->tokens == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append = 0;
    cmd->next = NULL;

    return cmd;
}

/* ---------- Add Token to Command ---------- */

void addCommandToken(Command *cmd, Token *token)
{
    if (cmd->count == cmd->capacity)
    {
        cmd->capacity *= 2;

        Token **temp = realloc(
            cmd->tokens,
            cmd->capacity * sizeof(Token *)
        );

        if (temp == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        cmd->tokens = temp;
    }

    cmd->tokens[cmd->count++] = token;
}

/* ---------- Parser ---------- */

Command *parse(Token *tokens)
{
    Command *head = NULL;
    Command *tail = NULL;

    Command *currentCommand = createCommand();

    Token *current = tokens;

    while (current != NULL &&
           current->type != TOKEN_END)
    {
        if (current->type == TOKEN_PIPE)
        {
            if (currentCommand->count == 0)
            {
                printf("Syntax Error near '|'.\n");
                return NULL;
            }

            if (head == NULL)
            {
                head = currentCommand;
                tail = currentCommand;
            }
            else
            {
                tail->next = currentCommand;
                tail = currentCommand;
            }

            currentCommand = createCommand();
        }
        else if (current->type == TOKEN_REDIRECT_IN)
        {
            current = current->next;

            currentCommand->input_file =
                duplicateString(current->value);
        }
        else if (current->type == TOKEN_REDIRECT_OUT)
        {
            current = current->next;

            currentCommand->output_file =
                duplicateString(current->value);

            currentCommand->append = 0;
        }
        else if (current->type == TOKEN_APPEND)
        {
            current = current->next;

            currentCommand->output_file =
                duplicateString(current->value);

            currentCommand->append = 1;
        }
        else
        {
            addCommandToken(currentCommand, current);
        }

        current = current->next;
    }

    /* Add final command */
    if (currentCommand->count > 0)
    {
        if (head == NULL)
        {
            head = currentCommand;
            tail = currentCommand;
        }
        else
        {
            tail->next = currentCommand;
            tail = currentCommand;
        }
    }
    else
    {
        free(currentCommand->tokens);
        free(currentCommand);
    }

    return head;
}

/* ---------- Print Parse Tree ---------- */

void printParseTree(Command *head)
{
    printf("\n========== PARSE TREE ==========\n");

    int commandNumber = 1;

    while (head != NULL)
    {
        printf("Command %d\n", commandNumber);

        printf("  Arguments:\n");

        for (int i = 0; i < head->count; i++)
        {
            printf("    └── %s\n",
                   head->tokens[i]->value);
        }

        if (head->input_file != NULL)
        {
            printf("  Input  : %s\n",
                   head->input_file);
        }

        if (head->output_file != NULL)
        {
            printf("  Output : %s (%s)\n",
                   head->output_file,
                   head->append ? "append" : "overwrite");
        }

        if (head->next != NULL)
        {
            printf("  |\n");
            printf("  v\n");
        }

        head = head->next;
        commandNumber++;
    }

    printf("================================\n");
}

/* ---------- Execution Structure ---------- */

void printExecutionStructure(Command *head)
{
    printf("\n======= EXECUTION STRUCTURE =======\n");

    int commandNumber = 1;

    while (head != NULL)
    {
        printf("Command %d: ", commandNumber);

        for (int i = 0; i < head->count; i++)
        {
            printf("%s ", head->tokens[i]->value);
        }

        if (head->input_file)
            printf("< %s ",
                   head->input_file);

        if (head->output_file)
        {
            if (head->append)
                printf(">> %s ",
                       head->output_file);
            else
                printf("> %s ",
                       head->output_file);
        }

        printf("\n");

        head = head->next;
        commandNumber++;
    }

    printf("===================================\n");
}

/* ---------- Free Parse Tree ---------- */

void freeCommands(Command *head)
{
    while (head != NULL)
    {
        Command *temp = head;

        free(head->tokens);
        free(head->input_file);
        free(head->output_file);

        head = head->next;

        free(temp);
    }
}

/* ---------- Free Tokens ---------- */

void freeTokens(Token *head)
{
    while (head != NULL)
    {
        Token *temp = head;

        head = head->next;

        free(temp->value);
        free(temp);
    }
}

/* ---------- Main ---------- */

int main(void)
{
    char input[MAX_INPUT];

    printf("Simple Command Parser\n");
    printf("Supported: |  <  >  >>\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        printf("parser> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        /* Handle empty command */
        if (strlen(input) == 0)
        {
            printf("Empty command. Nothing to parse.\n\n");
            continue;
        }

        if (strcmp(input, "exit") == 0)
            break;

        /* Tokenization */
        Token *tokens = tokenize(input);

        /* Debug output */
        printTokens(tokens);

        /* Validate token stream */
        if (!validateTokens(tokens))
        {
            freeTokens(tokens);
            printf("\n");
            continue;
        }

        /* Parser */
        Command *parseTree = parse(tokens);

        if (parseTree == NULL)
        {
            printf("Parsing failed.\n");
            freeTokens(tokens);
            continue;
        }

        /* Display parse tree */
        printParseTree(parseTree);

        /* Display execution structure */
        printExecutionStructure(parseTree);

        freeCommands(parseTree);
        freeTokens(tokens);

        printf("\n");
    }

    printf("Parser terminated successfully.\n");

    return 0;
}
