int contains(char *arr[], int size, char *str);
void parseGator(char *str);
int make_evaluations(char *path, char *error_message, char *tokens[], int pathSet, int num_strings, char fullPath[], char *paths[], int num_paths);
void singlePathFork(char fullPath[100], char path[20], char *tokens[20], int num_strings);
void multiplePathFork(int num_paths, char fullPath[100], char *paths[20], char *tokens[20], int num_strings, char error_message[50]);
void multipleExecs(char fullPath[100], char *tokens[], int num_strings);
void parseAnd(char *str[], int num_strings, char path[], char fullPath[]);
void parseAmpersand(char *str, char path[]);
void parseShGator(char *tokens[], char *str, int num_strings, int num_paths, char *paths[]);
