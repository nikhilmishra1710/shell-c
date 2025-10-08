#include "include/redirection.h"
#include "include/constants.h"
#include "include/utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int output_redirection(string* tokens[MAX_CMDS], int token_count, string* outfile_name,
                       int* outfile_mode, string* errfile_name, int* errfile_mode) {
    string* buffer_tokens[MAX_CMDS];
    int k = 0;
    for (int i = 0; i < token_count; i++) {
        if (strcmp(tokens[i]->chars, ">") == 0 || strcmp(tokens[i]->chars, "1>") == 0) {
            copy_string(outfile_name, tokens[++i]);
            *outfile_mode = 1;
        } else if (strcmp(tokens[i]->chars, ">>") == 0 || strcmp(tokens[i]->chars, "1>>") == 0) {
            copy_string(outfile_name, tokens[++i]);
            *outfile_mode = 2;
        } else if (strcmp(tokens[i]->chars, "2>") == 0) {
            copy_string(errfile_name, tokens[++i]);
            *errfile_mode = 1;
        } else if (strcmp(tokens[i]->chars, "2>>") == 0) {
            copy_string(errfile_name, tokens[++i]);
            *errfile_mode = 2;
        } else {
            buffer_tokens[k++] = tokens[i];
        }
    }
    printf("After Output Redirection\n");
    printf("Outfile: %s mode: %d\n", outfile_name->chars, *outfile_mode);
    printf("Errfile: %s mode: %d\n", errfile_name->chars, *errfile_mode);
    for (int i = 0; i < k; i++) {
        printf("%d: %s\n", i, buffer_tokens[i]->chars);
        tokens[i] = buffer_tokens[i];
    }

    return k;
}

void setup_output_redirection(string* outfile_name, int outfile_mode, string* errfile_name,
                              int errfile_mode) {
    if (outfile_mode) {
        int fd = open(outfile_name->chars,
                      O_WRONLY | O_CREAT | (outfile_mode == 1 ? O_TRUNC : O_APPEND), 0644);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else {
            perror("Stdout open");
        }
    }

    if (errfile_mode) {
        int fd = open(errfile_name->chars,
                      O_WRONLY | O_CREAT | (outfile_mode == 1 ? O_TRUNC : O_APPEND), 0644);
        if (fd >= 0) {
            dup2(fd, STDERR_FILENO);
            close(fd);
        } else {
            perror("Stderr open");
        }
    }
}

void restore_redirection(int saved_stdout, int saved_stderr) {
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stdout);
    close(saved_stderr);
}