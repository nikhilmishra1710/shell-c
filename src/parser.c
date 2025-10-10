#include "include/parser.h"
#include "include/constants.h"
#include "include/utils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void finish_token(char *buffer, int *buf_idx, string **tokens, int *token_count)
{
    if (*buf_idx > 0)
    {

        buffer[*buf_idx] = '\0';
        string* temp = allocate_string_chars(buffer);

        tokens[(*token_count)++] = temp;

        *buf_idx = 0;
    }
}

int pipe_commands(string* input, string* cmds[MAX_CMDS]) {
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
    // printf("Pipe breakup\n");
    while (cmd && count < MAX_CMDS) {
        while (isspace((unsigned char) *cmd)) {
            cmd++;
        }
        cmds[count++] = allocate_string_chars(cmd);
        // printf("%d: %s\n", count-1, cmds[count-1]->chars);
        cmd = strtok(NULL, "|");
    }

    cmds[count] = NULL;
    free_string(working_cp);
    if (cmd) free(cmd);

    return count;
}

int tokenize(string *line, string *tokens[MAX_LENGTH])
{
    parse_state_e state = STATE_NORMAL;
    parse_state_e prev_state = STATE_NORMAL;
    char buffer[MAX_LENGTH];
    int buf_idx = 0;
    int token_count = 0;

    for (int i = 0; i < line->size; i++)
    {
        char c = line->chars[i];

        switch (state)
        {
        case STATE_NORMAL:
            if (c == '\\')
            {

                prev_state = STATE_NORMAL;
                state = STATE_ESCAPED;
            }
            else if (c == '\'')
            {

                state = STATE_IN_SINGLE_QUOTES;
            }
            else if (c == '\"')
            {

                state = STATE_IN_DOUBLE_QUOTES;
            }
            else if (isspace((unsigned char)c))
            {
                finish_token(buffer, &buf_idx, tokens, &token_count);
            }
            else
            {

                if (buf_idx < MAX_LENGTH - 1)
                {
                    buffer[buf_idx++] = c;
                }
            }
            break;

        case STATE_IN_SINGLE_QUOTES:
            if (c == '\'')
            {

                state = STATE_NORMAL;
            }
            else
            {

                if (buf_idx < MAX_LENGTH - 1)
                {
                    buffer[buf_idx++] = c;
                }
            }
            break;

        case STATE_IN_DOUBLE_QUOTES:
            if (c == '\"')
            {

                state = STATE_NORMAL;
            }
            else if (c == '\\')
            {

                prev_state = STATE_IN_DOUBLE_QUOTES;
                state = STATE_ESCAPED;
            }
            else
            {

                if (buf_idx < MAX_LENGTH - 1)
                {
                    buffer[buf_idx++] = c;
                }
            }
            break;

        case STATE_ESCAPED:

            if (prev_state == STATE_NORMAL)
            {
                switch (c)
                {
                case 'n':
                    buffer[buf_idx++] = 'n';
                    break;
                case 't':
                    buffer[buf_idx++] = 't';
                    break;
                case ' ':
                    buffer[buf_idx++] = ' ';
                    break;
                case '\'':
                    buffer[buf_idx++] = '\'';
                    break;
                case '\"':
                    buffer[buf_idx++] = '\"';
                    break;
                default:

                    if (buf_idx < MAX_LENGTH - 2)
                    {
                        buffer[buf_idx++] = '\\';
                        buffer[buf_idx++] = c;
                    }
                    break;
                }
            }
            else if (prev_state == STATE_IN_DOUBLE_QUOTES)
            {
                switch (c)
                {

                case 't':
                    buffer[buf_idx++] = '\t';
                    break;
                case '\\':
                    buffer[buf_idx++] = '\\';
                    break;
                case '\"':
                    buffer[buf_idx++] = '\"';
                    break;
                default:
                    if (buf_idx < MAX_LENGTH - 2)
                    {
                        buffer[buf_idx++] = '\\';
                        buffer[buf_idx++] = c;
                    }
                    break;
                }
            }
            state = prev_state;
            break;
        }
    }

    finish_token(buffer, &buf_idx, tokens, &token_count);

    return token_count;
}
