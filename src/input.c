#include "include/input.h"
#include "include/constants.h"
#include "include/history.h"
#include "include/terminal.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void print_prompt(void) {
    enable_raw_mode();
    printf("devsh> ");
    fflush(stdout);
}

static void refresh_input_line(string* cmd, int cursor_index) {
    printf("\rdevsh> %s", cmd->chars); // redraw prompt and string
    printf("\033[K");                  // clear to end of line
    // Move cursor to correct position
    int move_left = cmd->size - cursor_index;
    if (move_left > 0) {
        printf("\033[%dD", move_left); // move cursor left by diff
    }
    fflush(stdout);
}

static void handle_arrow_input(int* index, cmd_history_t* history_obj) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) == -1 || read(STDIN_FILENO, &seq[1], 1) == -1) {
        perror("Failed read");
        exit(ERROR_STATUS);
    }

    if (seq[0] == '[') {
        cmd_history_t* temp_obj = NULL;
        if (seq[1] == 'A') {
            temp_obj = get_previous_history();
            *index = temp_obj->cmd->size;
            copy_string(history_obj->cmd, temp_obj->cmd);
        } else if (seq[1] == 'B') {
            temp_obj = get_next_history();
            *index = temp_obj->cmd->size;
            copy_string(history_obj->cmd, temp_obj->cmd);
        } else if (seq[1] == 'C') {
            if (*index < history_obj->cmd->size) {
                printf("\033[C");
                (*index)++;
            } else {
                printf("\a");
            }
        } else if (seq[1] == 'D') {
            if (*index > 0) {
                printf("\033[D");
                (*index)--;
            } else {
                printf("\a");
            }
        } else if (seq[1] == '3') {
            char del;
            if (read(STDIN_FILENO, &del, 1) == -1) {
                perror("Failed fread");
                exit(ERROR_STATUS);
            }
            if (del == '~') {
                // Delete Key Pressed
                if(*index != history_obj->cmd->size){
                    update_string_at(history_obj->cmd, *index, del, true);
                }
            } 
        }
    }
}

string* read_input() {
    cmd_history_t* history_obj = create_new_history_obj();
    print_prompt();
    char c;
    int index = 0;
    while (true) {
        read(STDIN_FILENO, &c, 1);
        if (c == '\033') {
            handle_arrow_input(&index, history_obj);
        } else if (c == '\n') {
            printf("\n");
            break;
        } else if (c == 127) {
            if (index == history_obj->cmd->size) {
                if (update_string(history_obj->cmd, c, true)) {
                    index--;
                }
            } else {
                if (update_string_at(history_obj->cmd, index - 1, c, true))
                    index--;
            }
        } else {
            if (index == history_obj->cmd->size) {
                if (update_string(history_obj->cmd, c, false))
                    index++;
            } else {
                if (update_string_at(history_obj->cmd, index, c, false))
                    index++;
            }
        }
        refresh_input_line(history_obj->cmd, index);
    }

    return history_obj->cmd;
}