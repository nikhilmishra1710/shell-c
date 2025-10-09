#include "include/input.h"
#include "include/autocomplete.h"
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

static void handle_arrow_input(int* index, string* input_buffer) {
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
            copy_string(input_buffer, temp_obj->cmd);
        } else if (seq[1] == 'B') {
            temp_obj = get_next_history();
            *index = temp_obj->cmd->size;
            copy_string(input_buffer, temp_obj->cmd);
        } else if (seq[1] == 'C') {
            if (*index < input_buffer->size) {
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
                if(*index != input_buffer->size){
                    update_string_at(input_buffer, *index, del, true);
                }
            } 
        }
    }
}

string* read_input() {
    cmd_history_t* history_obj = create_new_history_obj();
    string* input_buffer = allocate_string();
    print_prompt();
    char c;
    int index = 0, double_tab = 0;
    while (true) {
        read(STDIN_FILENO, &c, 1);
        if (c == '\033') {
            handle_arrow_input(&index, input_buffer);
        } else if (c == '\n') {
            printf("\n");
            break;
        } else if (c == 127) {
            if (index == input_buffer->size) {
                if (update_string(input_buffer, c, true)) {
                    index--;
                }
            } else {
                if (update_string_at(input_buffer, index - 1, c, true))
                    index--;
            }
        } else if (c=='\t') {
            int partial_completion = 0;
            index = autocomplete(input_buffer, double_tab, &partial_completion);
            
            if(index > 0){
                if (partial_completion == 0) {
                    update_string(input_buffer, ' ', false);
                }
            } else if (index == 0) {
                index = input_buffer->size;
                double_tab = 0;
            } else {
                index = input_buffer->size;
                double_tab = 1;
            }
        } 
        else {
            if (index == input_buffer->size) {
                if (update_string(input_buffer, c, false))
                    index++;
            } else {
                if (update_string_at(input_buffer, index, c, false))
                    index++;
            }
            copy_string(history_obj->cmd, input_buffer);
        }
        refresh_input_line(input_buffer, index);
    }

    free_string(input_buffer);
    return history_obj->cmd;
}