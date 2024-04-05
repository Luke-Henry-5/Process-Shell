#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "functions.h"

int main(int argc, char *argv[])
{

    char error_message[50] = "An error has occurred\n";
    char path[20] = "/bin";
    char *paths[20];
    char *tempPaths[10];
    int num_paths = 0; // Reset number of paths
    paths[0] = strdup(path);
    char fullPath[100] = "";
    char concatenated[50];
    // 0 == false
    int pathSet = 0;
    FILE *file;
    int lineNumber = 1;
    char line[100];
    if (argc == 1)
    {
        file = stdin; // Assign stdin to file pointer
        printf("wish> ");
    }
    else if (argc == 2)
    {
        file = fopen(argv[1], "r");
        if (file == NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1); // Exit function if file opening fails
        }
    }
    if (argc > 2)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    // Maybe another file

    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Line formatting
        // Removes the new line character if present
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';
        // Removes excess space at the end
        if (strcmp(&line[strlen(line) - 1], " ") == 0)
            line[strlen(line) - 1] = '\0';
        // Check if line is empty
        int isEmpty = 1;
        for (int i = 0; i < strlen(line); i++)
        {
            if (line[i] != ' ')
            {
                isEmpty = 0;
                break;
            }
        }

        if (isEmpty)
        {
            continue;
        }

        // Mock line created for tokenization/parsing purposes
        char mockLine[100];
        strcpy(mockLine, line);
        // printf("Line: %s\n", line);

        if (argc == 1)
        {
            // printf("line%d: %s\n", lineNumber, line);
            printf("wish> ");
        } // else {
        // Tokenize the line and count the number of tokens
        char *tokens[20];
        int num_strings = 0;
        char *token = strtok(mockLine, " ");
        while (token != NULL)
        {
            tokens[num_strings++] = token;
            token = strtok(NULL, " ");
        }

        if ((strcmp(tokens[0], "exit") == 0) && num_strings == 2)
        {
            // make this call method in the errors.c file that accomplishes the same thing
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
        else if (strcmp(line, "exit") == 0)
            exit(0);

        else if (strcmp(tokens[0], "cd") == 0)
        {
            if (num_strings == 2)
            {
                chdir(tokens[1]);
            }
            else
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }

        else if (strcmp(tokens[0], "path") == 0)
        {
            pathSet = 1;
            num_paths = 0;
            if (num_strings == 2)
            {
                strcpy(path, tokens[1]);
            }
            else if (num_strings > 2)
            {
                for (int i = 1; i < num_strings; i++)
                {
                    paths[num_paths++] = strdup(tokens[i]); // Set provided paths
                }
                // for (int i = 0; i < num_paths; i++)
                // {
                //     printf("Path %d: %s\n", i, paths[i]);
                // }
            }

            else
            {
                strcpy(path, "");
            }
        }
        else
        {

            // int evaluation = make_evaluations(path, error_message, tokens, pathSet, num_strings);
            // printf("%d\n", evaluation);)

            int errorDetected = make_evaluations(path, error_message, tokens, pathSet, num_strings, fullPath, paths, num_paths);
            if(!errorDetected)
            {
                if (num_paths > 0)
                {
                    // printf("Multiple paths\n");
                    multiplePathFork(num_paths, fullPath, paths, tokens, num_strings, error_message);
                }
                else
                {
                    singlePathFork(fullPath, path, tokens, num_strings);
                }
            }
        }
        lineNumber++;
    }
    if (argc == 2)
    {
        fclose(file); // Close the file if opened
    }
    return 0;
}
