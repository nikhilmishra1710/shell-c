#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

typedef struct {
    char* chars;
    int size;
    int capacity;
} string;

string* allocate_string(void);
string* allocate_string_chars(char* chars);
bool update_string(string* str, char c, bool remove);
bool update_string_at(string* str, int index, char ch, bool remove);
void copy_string(string* dest, const string* src);
void reallocate_string(string* str);
int is_executable(const char* path);
char* find_in_path(const char* command);

#endif