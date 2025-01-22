#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef enum
{
    false,
    true
} bool;

int is_executable(const char *path) { return access(path, X_OK) == 0; }

char *find_in_path(char *command)
{
    char *path_env = getenv("PATH");

    if (path_env == NULL)
    {
        return NULL;
    }

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    static char full_path[1024];
    while (dir != NULL)
    {
        snprintf(full_path, 1024, "%s/%s", dir, command);
        if (is_executable(full_path))
        {
            free(path_copy);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

void fork_and_exec_cmd(char *full_path, int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execv(full_path, argv);
    }
    else if (pid == 0)
    {
        perror("fork");
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

int parse_input(char *input, char *argv[])
{
    int argc = 0, i = 0;
    bool single_quote_open = false, double_quote_open = false;
    char *arg_start = NULL;
    char token[1024] = "";

    while (input[i] != '\0')
    {
        if (input[i] == '"' && !double_quote_open && !single_quote_open)
        {
            double_quote_open = true;
            arg_start = input + i + 1;
        }
        else if (input[i] == '"' && double_quote_open && !single_quote_open)
        {
            double_quote_open = false;
            input[i] = '\0';
        }
        else
        if (input[i] == '\'' && !single_quote_open && !double_quote_open)
        {
            single_quote_open = true;
            arg_start = input + i + 1;
        }
        else if (input[i] == '\'' && single_quote_open && !double_quote_open)
        {
            single_quote_open = false;
            input[i] = '\0';
        }
        else if ((input[i] == ' ' || input[i] == '\0') && !single_quote_open && !double_quote_open)
        {
            if (strlen(token) > 0)
            {
                argv[argc++] = strdup(token);
                token[0] = '\0'; 
            }
        }
        else
        {
            if(input[i] == '\\' && !single_quote_open && !double_quote_open)
            {
                i++;
            }
            if(input[i] == '\0')
            {
                break;
            }
            char current_char[2] = {input[i], '\0'};
            strcat(token, current_char); 
        }
        i++;
    }
    if (strlen(token) > 0)
    {
        
        argv[argc++] = strdup(token);
        // printf("argc[%d]: %s\n", argc, argv[argc - 1]);
    }

    argv[argc] = NULL;
    // printf("argc: %d\n",argc);
    return argc;
}

void handle_input(char *input)
{
    char *argv[10];
    // printf("%s\n",input);
    int argc = parse_input(input, argv);
    if (strcmp(argv[0], "exit") == 0)
    {
        
        char *endptr;
        int exit_code = strtol(argv[1], &endptr, 10);
        exit(exit_code);
    }
    else if (strcmp(argv[0], "echo") == 0)
    {
        for(int i=1; i<argc; i++)
        {
            printf("%s ", argv[i]);
        }
        printf("\n");
    }
    else if (strcmp(argv[0], "type") == 0)
    {
        char builtins[][16] = {"exit", "echo", "type", "pwd", "cd"};
        bool found = false;
        for (int i = 0; i < sizeof(builtins); i++)
        {
            if (strcmp(argv[1], builtins[i]) == 0)
            {
                printf("%s is a shell builtin\n", argv[1]);
                return;
            }
        }

        char *path = find_in_path(argv[1]);
        if (path != NULL)
        {
            printf("%s is %s\n", argv[1], path);
        }
        else
        {
            printf("%s: not found\n", argv[1]);
        }
    }
    else if(strcmp(argv[0], "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd");
        }
    }
    else if(strcmp(argv[0], "cd") == 0)
    {
        char *dir = argv[1];
        if(strcmp(dir, "~") == 0)
        {
            dir = getenv("HOME");
        }
        if (chdir(dir) < 0)
        {
            printf("cd: %s: No such file or directory\n", dir);
        }

    }
    else
    {
        char *cmd_path = find_in_path(argv[0]);
        if (cmd_path)
        {
            fork_and_exec_cmd(cmd_path, argc, argv);
            return;
        }
        else
        {
            printf("%s: command not found\n", argv[0]);
        }
    }
}

int main()
{
    // Flush after every printf
    bool run = true;

    while (run)
    {
        setbuf(stdout, NULL);

        // Uncomment this block to pass the first stage
        printf("$ ");

        // Wait for user input
        char input[100];
        fgets(input, 100, stdin);

        // Remove trailing newline character
        input[strcspn(input, "\n")] = 0;

        // Resolve the user input
        handle_input(input);
    }

    return 0;
}
