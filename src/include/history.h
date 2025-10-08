#ifndef HISTORY_H
#define HISTORY_H

#include "utils.h"

typedef struct cmd_history_t {
    string* cmd;
    struct cmd_history_t* next;
    struct cmd_history_t* prev;
} cmd_history_t;

cmd_history_t* create_new_history_obj(void);
cmd_history_t* get_previous_history(void);
cmd_history_t* get_next_history(void);

#endif