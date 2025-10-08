#ifndef PARSER_H
#define PARSER_H

#include "constants.h"
#include "utils.h"

typedef enum {
    STATE_NORMAL,
    STATE_IN_SINGLE_QUOTES,
    STATE_IN_DOUBLE_QUOTES,
    STATE_ESCAPED,
} parse_state_e;

int pipe_commands(string* input, string* cmds[MAX_CMDS]);
int tokenize(string* input, string* tokens[MAX_LENGTH]);

#endif