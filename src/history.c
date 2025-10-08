#include "include/history.h"
#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

cmd_history_t* head = NULL;
cmd_history_t* current = NULL;

cmd_history_t* create_new_history_obj() {
    cmd_history_t* temp = (cmd_history_t*) malloc(sizeof(cmd_history_t));
    temp->cmd = allocate_string();
    temp->next = NULL;
    temp->prev = NULL;
    if (head) {
        head->next = temp;
        temp->prev = head;
        head = temp;
    } else {
        head = temp;
    }
    current = head;
    return head;
}

cmd_history_t* get_previous_history() {
    if (current->prev == NULL) {
        printf("\a");
    } else {
        current = current->prev;
    }
    return current;
}

cmd_history_t* get_next_history() {
    if (current->next == NULL) {
        printf("\a");

    } else {
        current = current->next;
    }
    return current;
}