#include "include/constants.h"
#include "include/execute.h"
#include "include/input.h"
#include "include/parser.h"
#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    string* input = NULL;
    string* cmds[MAX_CMDS];
    while (true) {
        input = read_input();
        printf("Input: %s\n", input->chars);
        int cmd_count = pipe_commands(input, cmds);
        printf("Cmd count: %d\n", cmd_count);
        execute_cmds(cmds, cmd_count);
    }

    return 0;
}