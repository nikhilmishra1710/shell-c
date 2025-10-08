#include "include/execute.h"
#include "include/builtins.h"
#include "include/constants.h"
#include "include/parser.h"
#include "include/redirection.h"
#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void fork_and_execute(char* cmd_path, char* args[MAX_CMDS], string* outfile_name,
                             int outfile_mode, string* errfile_name, int errfile_mode) {
    pid_t pid = fork();
    if (pid == 0) {
        setup_output_redirection(outfile_name, outfile_mode, errfile_name, errfile_mode);
        execv(cmd_path, args);
        perror("execv error");
        exit(ERROR_STATUS);
    } else if (pid < 1) {
        perror("Fork error");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

static void execute_cmd(string* input) {
    string* tokens[MAX_LENGTH];
    int outfile_mode = 0, errfile_mode = 0;
    string *outfile_name = allocate_string(), *errfile_name = allocate_string();
    int saved_stdout = dup(STDOUT_FILENO), saved_stderr = dup(STDERR_FILENO);

    int token_count = tokenize(input, tokens);

    for (int i = 0; i < token_count; i++) {
        printf("%d: %s\n", i, tokens[i]->chars);
    }

    token_count = output_redirection(tokens, token_count, outfile_name, &outfile_mode, errfile_name,
                                     &errfile_mode);

    char* args[MAX_LENGTH];
    for (int i = 0; i < token_count; i++) {
        strcpy(args[i], tokens[i]->chars);
    }
    int found = 0;
    int check = 0;

    execute_builtins(&check, &found, args, token_count, outfile_name, outfile_mode, errfile_name,
                     errfile_mode);
    if (!found) {
        char* cmd_path = find_in_path(args[0]);
        if (cmd_path != NULL) {
            args[token_count] = NULL;
            fork_and_execute(cmd_path, args, outfile_name, outfile_mode, errfile_name,
                             errfile_mode);
            found = 1;
        }
    }

    if (found == 0 || check != 0) {
        printf("%s: command not found\n", args[0]);
    }

    for (int i = 0; i < token_count; i++) {
        free(args[i]);
        free(tokens[i]);
    }

    restore_redirection(saved_stdout, saved_stderr);
}

void execute_cmds(string* cmd[MAX_CMDS], int cmd_count) {
    if (cmd_count == 0) {
        return;
    } else if (cmd_count == 1) {
        execute_cmd(cmd[cmd_count - 1]);
    } else {
        for (int i = 0; i < cmd_count; i++) {
            printf("%d: %s\n", i, cmd[i]->chars);
        }
    }
}