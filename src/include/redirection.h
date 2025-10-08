#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "constants.h"
#include "utils.h"

int output_redirection(string* tokens[MAX_CMDS], int token_count, string* outfile_name,
                       int* outfile_mode, string* errfile_name, int* errfile_mode);
void setup_output_redirection(string* outfile_name, int outfile_mode, string* errfile_name,
                              int errfile_mode);
void restore_redirection(int saved_stdout, int saved_stderr);

#endif