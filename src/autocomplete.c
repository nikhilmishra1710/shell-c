#define _POSIX_C_SOURCE 200809L
#include "include/autocomplete.h"
#include "include/builtins.h"
#include "include/utils.h"
#include <dirent.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static bool is_duplicate(char possible[][PATH_MAX], int count, char* name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(possible[i], name) == 0) {
            return true;
        }
    }
    return false;
}

static int compare_strings(const void* a, const void* b) {
    return strcmp((const char*) a, (const char*) b);
}

static void longest_common_prefix(char strings[][PATH_MAX], int count, char* result) {
    if (count == 0) {
        result[0] = '\0';
        return;
    }

    int index = 0;
    while (1) {
        char current_char = strings[0][index];
        for (int i = 1; i < count; i++) {
            if (strings[i][index] != current_char || strings[i][index] == '\0') {
                result[index] = '\0';
                return;
            }
        }
        result[index] = current_char;
        index++;
    }
}

int autocomplete(string* input, int double_tab, int* partial_completion) {
    if (double_tab) {
        printf("\n");
    }
    int found = 0, idx = -1;
    for (int i = 0; i < get_builtin_command_size(); i++) {
        if ((strncmp(input->chars, get_builtin_command(i).name, (size_t)input->size) == 0)) {
            found = 1;
            idx = i;
        }
    }

    if (found == 1) {
        string* temp = allocate_string_chars(get_builtin_command(idx).name);
        copy_string(input, temp);
        free(temp);
        return input->size;
    }

    char* path_env = getenv("PATH");

    if (path_env == NULL)
        return -1;
    char* path_copy = strdup(path_env);
    char* dir = strtok(path_copy, ":");

    char possible[1000][PATH_MAX];
    int possible_count = 0;

    while (dir != NULL) {
        DIR* dp = opendir(dir);
        if (dp == NULL) {
            dir = strtok(NULL, ":");
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dp)) != NULL) {
            // Check if the file name starts with input
            if (strncmp(entry->d_name, input->chars, (size_t) input->size) == 0) {
                // Construct the full path
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);

                // Check if it's executable
                if (access(full_path, X_OK) == 0 &&
                    !is_duplicate(possible, possible_count, entry->d_name)) {
                    strcpy(possible[possible_count++], entry->d_name);
                }
            }
        }

        closedir(dp);
        dir = strtok(NULL, ":");
    }

    if (possible_count == 0) {
        printf("\a"); // Bell sound for no match
        free(path_copy);
        return -1;
    }

    if (possible_count == 1) {
        // printf("Partial completion: %u\n", *partial_completion);
        string* temp = allocate_string_chars(possible[0]);
        copy_string(input, temp);
        free(temp);
        free(path_copy);
        return input->size;
    }

    // Find the longest common prefix
    char lcp[PATH_MAX];
    longest_common_prefix(possible, possible_count, lcp);

    if ((int) strlen(lcp) > input->size) {
        string* temp = allocate_string_chars(lcp);
        copy_string(input, temp);
        free(temp);
        free(path_copy);
        *partial_completion = 1;
        return input->size;
    }

    // On double tab, show all possibilities
    if (double_tab) {
        qsort(possible, (size_t) possible_count, PATH_MAX, compare_strings);
        for (int i = 0; i < possible_count; i++) {
            printf("%s  ", possible[i]);
        }
        printf("\n");
        return 0;
    }

    free(path_copy);
    return -1;
}