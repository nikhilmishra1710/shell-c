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

void handle_input(char *input)
{
    if (strncmp(input, "exit ", 5) == 0)
    {
        char *arguments = input + 5;
        char *endptr;
        int exit_code = strtol(arguments, &endptr, 10);
        exit(exit_code);
    }
    else if (strncmp(input, "echo ", 5) == 0)
    {
        printf("%s\n", input + 5);
    }
    else if (strncmp(input, "type ", 5) == 0)
    {
        char builtins[][16] = {"exit", "echo", "type", "pwd", "cd"};
        char *arguments = input + 5;
        bool found = false;
        for (int i = 0; i < sizeof(builtins); i++)
        {
            if (strcmp(arguments, builtins[i]) == 0)
            {
                printf("%s is a shell builtin\n", arguments);
                return;
            }
        }

        char *path = find_in_path(arguments);
        if (path != NULL)
        {
            printf("%s is %s\n", arguments, path);
        }
        else
        {
            printf("%s: not found\n", arguments);
        }
    }
    else if(strcmp(input, "pwd") == 0)
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
    else if(strncmp(input, "cd ", 3) == 0)
    {
        char *dir = input + 3;
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
        char *argv[10];
        int argc = 0;
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            argv[argc] = token;
            token = strtok(NULL, " ");
            argc++;
        }
        argv[argc] = NULL;
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
