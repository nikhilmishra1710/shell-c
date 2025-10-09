#include "include/execute.h"
#include "include/builtins.h"
#include "include/constants.h"
#include "include/parser.h"
#include "include/redirection.h"
#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>
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
    int saved_stdout = dup(STDOUT_FILENO), saved_stderr = dup(STDERR_FILENO);
    string *outfile_name = allocate_string(), *errfile_name = allocate_string();

    int token_count = tokenize(input, tokens);

    for (int i = 0; i < token_count; i++) {
        printf("%d: %s\n", i, tokens[i]->chars);
    }

    token_count = output_redirection(tokens, token_count, outfile_name, &outfile_mode, errfile_name,
                                     &errfile_mode);

    char* args[MAX_LENGTH];
    printf("token_count: %d\n", token_count);
    for (int i = 0; i < token_count; i++) {
        printf("i: %d %s\n", i, tokens[i]->chars);
        args[i] = tokens[i]->chars;
        printf("args[%d]: %s\n", i, args[i]);
    }
    printf("Starting execution\n");
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
        free_string(tokens[i]);
    }

    free_string(outfile_name);
    free_string(errfile_name);
    restore_redirection(saved_stdout, saved_stderr);
}

static void execute_pipe_cmd(string** cmds, int cmd_count) {
    int pipefd[2];
    int prev_pipe_read = STDIN_FILENO;
    pid_t pids[cmd_count];

    for (int i = 0; i < cmd_count; i++) {
        // Create a pipe if this is not the last command
        if (i < cmd_count - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        if (pid == 0) {
            // ----- CHILD -----

            // If not the first command, read from previous pipe
            if (i > 0) {
                if (dup2(prev_pipe_read, STDIN_FILENO) < 0) {
                    perror("dup2 prev_pipe_read");
                    exit(EXIT_FAILURE);
                }
            }

            // If not the last command, write to next pipe
            if (i < cmd_count - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    perror("dup2 pipe write");
                    exit(EXIT_FAILURE);
                }
            }

            // Close unneeded FDs in the child
            if (i < cmd_count - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            if (prev_pipe_read != STDIN_FILENO)
                close(prev_pipe_read);

            printf("cmd[%d]: %s\n", i, cmds[i]->chars);
            execute_cmd(cmds[i]); // Run the command
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else {
            // ----- PARENT -----
            pids[i] = pid;

            // Close write-end of current pipe in parent
            if (i < cmd_count - 1) {
                close(pipefd[1]);
            }

            // Close previous pipe (not needed anymore)
            if (prev_pipe_read != STDIN_FILENO) {
                close(prev_pipe_read);
            }

            // Next command should read from this pipe
            if (i < cmd_count - 1) {
                prev_pipe_read = pipefd[0];
            }
        }
    }

    // ----- PARENT: Wait for all children -----
    for (int i = 0; i < cmd_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
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
        execute_pipe_cmd(cmd, cmd_count);
    }

    for(int i=0; i<cmd_count; i++) {
        free_string(cmd[i]);
    }
}