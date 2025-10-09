#ifndef BUILTINS_H
#define BUILTINS_H

#include "utils.h"

typedef struct builtin_command_t {
    const char* name;
    int (*process)(char* args[], int argc, string* outfile_name, int outfile_mode,
                   string* errfile_name, int errfile_mode);
} builtin_command_t;

int process_exit(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode);
int process_echo(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode);
int process_type(char* args[], int argc, string* outfile_name, int outfile_mode,
                 string* errfile_name, int errfile_mode);
int process_pwd(char* args[], int argc, string* outfile_name, int outfile_mode,
                string* errfile_name, int errfile_mode);
int process_cd(char* args[], int argc, string* outfile_name, int outfile_mode, string* errfile_name,
               int errfile_mode);
int get_builtin_command_size(void);
builtin_command_t get_builtin_command(int i);
void execute_builtins(int* check, int* found, char** args, int token_num, string* outfile_name,
                      int outfile_mode, string* errfile_name, int errfile_mode);
#endif