#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int contains(char *arr[], int size, char *str)
{
    for (int i = 0; i < size; i++)
    {
        if (strstr(arr[i], str) != NULL || strchr(arr[i], '>') != NULL)
        {
            return 1; // String found
        }
    }
    return 0; // String not found
}

void parseGator(char *str)
{
    // Find the position of '>'
    char *separator = strchr(str, '>');
    if (separator == NULL)
    {
        printf("Separator '>' not found in input string.\n");
        return;
    }

    // Calculate the length of the left and right substrings
    int leftLength = separator - str;
    int rightLength = strlen(separator + 1);

    // Define fixed-size arrays for left and right substrings
    char leftStr[leftLength + 1];
    char rightStr[rightLength + 1];

    strncpy(leftStr, str, leftLength);
    leftStr[leftLength] = '\0';

    strcpy(rightStr, separator + 1);

    // Execute the ls function on the right path and store it in the right location
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        int output_fd = open(rightStr, O_WRONLY | O_TRUNC | O_CREAT, 0600); // Open the output file
        dup2(output_fd, 1);                                                 // Redirect stdout to the file descriptor of the output file
        close(output_fd);                                                   // Close the output file descriptor after redirection

        char *args[] = {"ls", leftStr, NULL}; // Command and its arguments
        execv("/bin/ls", args);

        // If execv returns, it means an error occurred
        perror("execv");
        exit(0); // Exiting the child process on execv failure
    }
    wait(NULL);
}

void parseShGator(char *tokens[], char *str, int num_strings, int num_paths, char *paths[])
{
    char concatenation[100] = "";

    for (int i = 0; i < num_strings; i++)
    {
        if (strlen(concatenation) + strlen(tokens[i]) >= 100 - 1)
        {
            fprintf(stderr, "Error: Resulting string exceeds maximum length.\n");
            return;
        }

        strcat(concatenation, tokens[i]);
    }

    const char *delimiter = "&";
    char *command;
    int index = 0;
    char *commands[100];

    command = strtok(concatenation, delimiter);
    while (command != NULL)
    {
        commands[index++] = command;
        command = strtok(NULL, delimiter);
    }

    char fullPath[100];
    pid_t child_pids[100]; // Array to store child process IDs
    int num_children = 0;

    for (int i = 0; i < index; i++)
    {
        char *token;
        token = strtok(commands[i], ">");
        char *commandParts[2];
        int j = 0;
        while (token != NULL)
        {
            commandParts[j++] = token;
            token = strtok(NULL, ">");
        }

        for (int k = 0; k < num_paths; k++)
        {
            strcpy(fullPath, paths[k]);
            strcat(fullPath, "/");
            strcat(fullPath, commandParts[0]);

            if (access(fullPath, X_OK) != -1)
            {
                pid_t pid;
                pid = fork();
                if (pid == -1)
                    perror("fork");
                else if (pid == 0)
                {
                    int output_fd = open(commandParts[1], O_WRONLY | O_TRUNC | O_CREAT, 0600);
                    dup2(output_fd, 1);
                    close(output_fd);

                    char *args[] = {fullPath, NULL};
                    execv(fullPath, args);

                    perror("execv");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    num_children++;
                }
            }
        }
    }

    // Wait for all child processes to finish executing
    for (int i = 0; i < num_children; i++)
    {
        wait(NULL);
    }
}

void singlePathFork(char fullPath[100], char path[20], char *tokens[20], int num_strings)
{
    pid_t pid;
    pid = fork();
    if (pid == -1)
        perror("fork");
    else if (pid == 0)
    {
        // Child process
        strcpy(fullPath, path);
        strcat(fullPath, "/");
        strcat(fullPath, tokens[0]);

        // Prepare arguments for execv
        char *args[num_strings + 1]; // +1 for NULL terminator
        for (int i = 0; i < num_strings; i++)
        {
            args[i] = tokens[i];
        }
        args[num_strings] = NULL; // NULL terminator

        // printf("%s\n", fullPath);
        // printf("executing\n");
        execv(fullPath, args);
    }
    else
    {
        wait(NULL);
    }
}

void multipleExecs(char fullPath[100], char *commands[], int num_strings)
{
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // Child process
        char *args[num_strings + 1]; // +1 for NULL terminator
        for (int i = 0; i < num_strings; i++)
        {
            args[i] = commands[i];
        }
        args[num_strings] = NULL; // NULL terminator
        execv(fullPath, args);
        perror("execv"); // execv only returns if an error occurs
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void parseAmpersand(char *str, char path[])
{
    const int MAX_TOKENS = 100;

    // Define an array of pointers to store the tokens
    char *tokens[MAX_TOKENS];

    // Split the input string based on '&'
    char *token = strtok(str, "&");
    int num_tokens = 0;
    while (token != NULL && num_tokens < MAX_TOKENS)
    {
        // Trim leading and trailing spaces from the token
        while (*token == ' ')
            token++;
        char *end = token + strlen(token) - 1;
        while (*end == ' ' && end >= token)
            end--;
        *(end + 1) = '\0';

        // Store the token in the array
        tokens[num_tokens++] = token;

        // Get the next token
        token = strtok(NULL, "&");
    }

    // Print the tokens
    // printf("Parsed tokens:\n");
    for (int i = 0; i < num_tokens; i++)
    {
        // printf("%s\n", tokens[i]);
        char fullPath[100];
        strcpy(fullPath, path);
        strcat(fullPath, "/");
        strcat(fullPath, tokens[i]);
        multipleExecs(fullPath, tokens, num_tokens);
    }
}

void parseAnd(char *tokens[], int num_strings, char path[], char fullPath[])
{
    // Count the number of executable commands
    int execsNumber = 0;
    for (int i = 0; i < num_strings; i++)
    {
        if (strcmp("&", tokens[i]) != 0)
        {
            execsNumber++;
        }
    }
    char **execs = malloc(execsNumber * sizeof(char *));
    // if (execs == NULL) {
    //     perror("malloc");
    //     exit(EXIT_FAILURE);
    // }
    int execAssigner = 0;
    for (int i = 0; i < num_strings; i++)
    {
        if (strcmp("&", tokens[i]) != 0)
        {
            // strncpy(execs[execAssigner], tokens[i], 20);
            execs[execAssigner] = tokens[i];
            // execs[execAssigner] = strdup(tokens[i]);
            // if (execs[execAssigner] == NULL) {
            //     perror("strdup");
            //     exit(EXIT_FAILURE);
            // }
            execAssigner++;
        }
    }

    for (int i = 0; i < execsNumber; i++)
    {
        char fullPath[100];
        strcpy(fullPath, path);
        strcat(fullPath, "/");
        strcat(fullPath, execs[i]);
        // printf("Command: %s\n", execs[i]);
        // printf("Fullpath: %s\n", fullPath);
        // printf("Num Strings: %d\n", execsNumber);
        multipleExecs(fullPath, execs, execsNumber);
    }
}

int make_evaluations(char *path, char *error_message, char *tokens[], int pathSet, int num_strings, char fullPath[], char *paths[], int num_paths)
{
    int errorDetected = 0;
    if (strcmp(path, "") == 0)
    {
        // printf("ONE\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        errorDetected = 1;
    }
    else if ((strstr(tokens[0], ".sh") != NULL) && !pathSet)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        errorDetected = 1;
    }
    else if ((strstr(tokens[0], ".sh") != NULL) && contains(tokens, num_strings, ">") && contains(tokens, num_strings, "&"))
    {
        parseShGator(tokens, "&", num_strings, num_paths, paths);
        errorDetected = 1;
    }
    else if (strcmp(tokens[0], ">") == 0)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        errorDetected = 1;
    }
    else if ((num_strings > 1) && (strcmp(tokens[0], "ls") == 0) && (strcmp(tokens[1], ">") == 0) && (num_strings != 3))
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        errorDetected = 1;
        // printf("ONE\n");
    }
    else if (contains(tokens, num_strings, "ls>") && (num_strings != 2))
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        errorDetected = 1;
        // printf("THREE\n");
    }
    else if ((strcmp(tokens[0], "ls") == 0) && contains(tokens, num_strings, ">") && (num_strings == 2))
    {
        parseGator(tokens[1]);
        errorDetected = 1;
        //  printf("TWO\n");
    }
    else if (num_strings > 1 && contains(tokens, num_strings, "&"))
    {
        parseAnd(tokens, num_strings, path, fullPath);
        errorDetected = 1;
    }
    else if (num_strings == 1 && contains(tokens, num_strings, "&"))
    {
        parseAmpersand(tokens[0], path);
        errorDetected = 1;
    }
    return errorDetected;
}

void multiplePathFork(int num_paths, char fullPath[100], char *paths[20], char *tokens[20], int num_strings, char error_message[50])
{
    int j = 0;
    // printf("numpath: %d\n", num_paths);
    for (int i = 0; i < num_paths; i++)
    {
        strcpy(fullPath, paths[i]);
        strcat(fullPath, "/");
        strcat(fullPath, tokens[0]);
        char *args[num_strings + 1];
        for (int i = 0; i < num_strings; i++)
        {
            args[i] = tokens[i];
        }
        args[num_strings] = NULL;

        if (access(fullPath, F_OK) != -1)
        {
            pid_t pid;
            pid = fork();
            if (pid == -1)
                perror("fork");
            else if (pid == 0)
            {
                execv(fullPath, args);
            }
            else
            {
                wait(NULL);
            }
        }
        else
        {
            j++;
            if (j == num_paths)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        // printf("%s\n", fullPath);
    }
    j = 0;
}
