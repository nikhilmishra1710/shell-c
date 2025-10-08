#include "include/parser.h"
#include "include/constants.h"
#include "include/utils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pipe_commands(string* input, string* cmds[]) {
    char* cmd;
    int count = 0;
    bool double_quote = false, single_quote = false;
    string* working_cp = allocate_string();
    copy_string(working_cp, input);
    for (int i = 0; i < working_cp->size; i++) {
        if (working_cp->chars[i] == '"') {
            double_quote = !double_quote;
        }
        if (working_cp->chars[i] == '\'') {
            single_quote = !single_quote;
        }
        if (working_cp->chars[i] == '|' && (double_quote || single_quote)) {
            working_cp->chars[i] = ' ';
        }
    }

    cmd = strtok(working_cp->chars, "|");

    while (cmd && count < MAX_CMDS) {
        while (isspace((unsigned char) *cmd)) {
            cmd++;
        }
        printf("%s\n", cmd);
        cmds[count++] = allocate_string_chars(cmd);
        cmd = strtok(NULL, "|");
    }

    cmds[count] = NULL;
    free(working_cp);

    return count;
}