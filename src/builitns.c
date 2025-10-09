#include "include/builtins.h"
#include "include/constants.h"
#include "include/redirection.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

builtin_command_t builtin_commands[] = {
    {"exit", process_exit}, {"echo", process_echo}, {"type", process_type},
    {"pwd", process_pwd},   {"cd", process_cd},
};

int process_exit(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode) {
    (void) outfile_name;
    (void) outfile_mode;
    (void) errfile_name;
    (void) errfile_mode;
    if (argc == 2) {
        if (strncmp(args[1], "0", 1) == 0) {
            exit(EXIT_SUCCESS);
        } else {
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Incorrect use of exit\n");
        return 1;
    }
    return 0;
}

int process_echo(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode) {
    setup_output_redirection(outfile_name, outfile_mode, errfile_name, errfile_mode);
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1) {
            printf("%s ", args[i]);
        } else {
            printf("%s", args[i]);
        }
    }
    printf("\n");
    return 0;
}

int process_type(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode) {
    (void) outfile_name;
    (void) outfile_mode;
    (void) errfile_name;
    (void) errfile_mode;
    if (argc == 2) {
        for (int i = 0; i < (int) (ARRAY_SIZE(builtin_commands)); i++) {
            if (strncmp(args[1], builtin_commands[i].name, strlen(builtin_commands[i].name)) == 0) {
                printf("%s is a shell builtin\n", builtin_commands[i].name);
                return 0;
            }
        }

        char* path = find_in_path(args[1]);
        if (path != NULL) {
            printf("%s is %s\n", args[1], path);
            return 0;
        }
        printf("%s: not found\n", args[1]);
    }

    return 0;
}

int process_pwd(char* args[], int argc, string* outfile_name, int outfile_mode,
                string* errfile_name, int errfile_mode) {
    (void) outfile_name;
    (void) outfile_mode;
    (void) errfile_name;
    (void) errfile_mode;
    (void) argc;
    (void) args;
    char current_path[PATH_MAX];
    if (getcwd(current_path, PATH_MAX) != NULL) {
        printf("%s\n", current_path);
    } else {
        perror("getcwd");
    }
    return 0;
}

int process_cd(char* args[], int argc, string* outfile_name, int outfile_mode, string* errfile_name,
               int errfile_mode) {
    (void) outfile_name;
    (void) outfile_mode;
    (void) errfile_name;
    (void) errfile_mode;
    printf("argc: %d\n", argc);
    if (argc == 1) {
        char* home_env = getenv("HOME");
        printf("home: %s\n", home_env);
        if (home_env == NULL)
            perror("HOME");
        if (chdir(home_env) != 0)
            perror("cd HOME");
    } else if (argc == 2 && strncmp(args[1], "~", 1) == 0) {
        char* home_env = getenv("HOME");
        if (home_env == NULL)
            perror("HOME");
        if (chdir(home_env) != 0)
            perror("cd HOME");
    } else if (argc == 2 && chdir(args[1]) != 0) {
        printf("cd: %s: No such file or directory\n", args[1]);
    } else if (argc > 2) {
        printf("Usage cd <path> (Only one path value is supported)");
    }
    return 0;
}

builtin_command_t get_builtin_command(int i) {
    return builtin_commands[i];
}

int get_builtin_command_size() {
    return ARRAY_SIZE(builtin_commands);
}

void execute_builtins(int* check, int* found, char** args, int token_num, string* outfile_name,
                      int outfile_mode, string* errfile_name, int errfile_mode) {
    for (int i = 0; i < (int) (ARRAY_SIZE(builtin_commands)); i++) {
        printf("builtin cmd: %s\n", builtin_commands[i].name);
        if (strncmp(args[0], builtin_commands[i].name, strlen(builtin_commands[i].name)) == 0) {
            *check = builtin_commands[i].process(args, token_num, outfile_name, outfile_mode,
                                                 errfile_name, errfile_mode);
            printf("Builtin execution complete\n");
            *found = 1;
        }
    }
}