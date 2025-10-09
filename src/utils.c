#define _POSIX_C_SOURCE 200809L
#include "include/utils.h"
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

string* allocate_string() {
    string* temp = (string*) malloc(sizeof(string));
    temp->size = 0;
    temp->capacity = 1024;
    temp->chars = (char*) malloc(sizeof(char) * (size_t) temp->capacity);
    if (temp->chars == NULL) {
        printf("Error in allocating string");
        exit(1);
    }
    temp->chars[0] = '\0';
    return temp;
}

string* allocate_string_chars(const char* chars) {
    if (chars == NULL)
        return allocate_string();
    string* temp = (string*) malloc(sizeof(string));
    temp->size = (int) strlen(chars);
    temp->capacity = 1024;
    while (temp->capacity < temp->size)
        temp->capacity *= 2;
    temp->chars = (char*) malloc(sizeof(char) * (size_t) temp->capacity);
    strcpy(temp->chars, chars);

    return temp;
}

bool update_string(string* str, char ch, bool remove) {
    if (!remove) {
        if (str->size + 1 > str->capacity) {
            printf("Reallocating str\n");
            reallocate_string(str);
        }
        str->chars[str->size++] = ch;
        str->chars[str->size] = '\0';
    } else {
        if (str->size <= 0) {
            printf("\a");
            return false;
        } else {
            str->chars[--str->size] = '\0';
        }
    }

    return true;
}

bool update_string_at(string* str, int index, char c, bool remove) {
    if (index < 0 || index >= str->size) {
        printf("Debug: idx: %d str->size: %d\n", index, str->size);
        return false;
    }
    if (remove) {
        if (str->size == 0) {
            printf("\a");
            return false;
        }
        memmove(&str->chars[index], &str->chars[index + 1], (size_t) (str->size - index + 1));
        str->size--;

    } else {
        if (str->size + 1 > str->capacity) {
            reallocate_string(str);
        }

        memmove(&str->chars[index + 1], &str->chars[index], (size_t) (str->size - index + 1));
        str->chars[index] = c;
        str->size++;
    }

    return true;
}

void copy_string(string* dest, const string* src) {
    if (!dest || !src)
        return;

    // Reallocate if needed
    if (dest->capacity < src->size + 1) {
        dest->capacity = src->size + 1;
        dest->chars = realloc(dest->chars, (size_t) (dest->capacity));
        if (!dest->chars) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }
    }

    memcpy(dest->chars, src->chars, (size_t) (src->size + 1)); // include '\0'
    dest->size = src->size;
}

void reallocate_string(string* str) {
    int oldCapacity = str->capacity;
    str->capacity = 2 * oldCapacity;
    str->chars = (char*) realloc(str->chars, sizeof(char) * (size_t) str->capacity);
}

void free_string(string* str) {
    if(!str) return;
    free(str->chars);
    free(str);
}

int is_executable(const char* path) {
    return access(path, X_OK) == 0;
}

char* find_in_path(const char* command) {
    char* path_env = getenv("PATH");
    if (path_env == NULL)
        return NULL;

    char* path_copy = strdup(path_env);
    char* dir = strtok(path_copy, ":");

    static char full_path[PATH_MAX];

    while (dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        if (is_executable(full_path) == 1) {
            free(path_copy);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}