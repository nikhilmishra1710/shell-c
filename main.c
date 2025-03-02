#include <ctype.h>
#include <limits.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#define MAX_TOKENS 100
#define MAX_LENGTH 256

void finish_token(char *buffer, int *buf_idx, char **tokens, int *token_count);
void execute_cmd(char *input);
int tokenize_cmd(const char *input, char *cmds[MAX_TOKENS]);
int tokenize(const char *line, char **tokens);
int is_executable(const char *path);
char *find_in_path(const char *command);
void fork_and_execute(char *cmd_path, int argc, char **args, char *out_file, int out_file_type, int mode);
void execute_pipe_cmd(char **cmds, int cmd_count);
int process_exit(char *args[], int argc, char *out_file, int out_file_type, int mode);
int process_echo(char *args[], int argc, char *out_file, int out_file_type, int mode);
int process_type(char *args[], int argc, char *out_file, int out_file_type, int mode);
int process_pwd(char *args[], int argc, char *out_file, int out_file_type, int mode);
int process_cd(char *args[], int argc, char *out_file, int out_file_type, int mode);
void disable_raw_mode();
void enable_raw_mode();
int autocomplete(char *input, int double_tab, int *partial_completion);
bool is_duplicate(char possible[][PATH_MAX], int count, char *name);
int compare_strings(const void *a, const void *b);
void longest_common_prefix(char strings[][PATH_MAX], int count, char *result);

struct termios orig_termios;

typedef enum
{
    STATE_NORMAL,
    STATE_IN_SINGLE_QUOTES,
    STATE_IN_DOUBLE_QUOTES,
    STATE_ESCAPED,
} parse_state_e;

typedef struct builtin_command_t
{
    const char *name;
    int (*process)(char *args[], int argc, char *out_file, int out_file_type, int mode);
} builtin_command_t;

typedef struct cmd_history_t
{
    char *cmd;
    struct cmd_history_t *next;
    struct cmd_history_t *prev;
} cmd_history_t;

builtin_command_t builtin_commands[] = {
    {"exit", process_exit},
    {"echo", process_echo},
    {"type", process_type},
    {"pwd", process_pwd},
    {"cd", process_cd},
};

int main()
{

    char input[1024];
    int index = 0, edit_index = 0;
    char *cmd[MAX_TOKENS];
    int double_tab = 0;
    cmd_history_t *curr = NULL, *tail = NULL;
    while (1)
    {

        // Flush after every printf
        enable_raw_mode();
        setbuf(stdout, NULL);
        printf("$ ");
        index = 0, edit_index = 0;
        memset(input, 0, 1024);
        curr = (cmd_history_t *)malloc(sizeof(cmd_history_t));
        if (curr == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        curr->cmd = (char *)malloc(1024);
        if (curr->cmd == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if (tail != NULL)
        {
            curr->next = NULL;
            tail->next = curr;
            curr->prev = tail;
            tail = curr;
        }
        else
        {
            curr->next = NULL;
            curr->prev = NULL;
            tail = curr;
        }

        while (1)
        {
            // printf("Loop: %d %d\n", index, edit_index);
            strcpy(curr->cmd, input);
            char c;
            if (read(STDIN_FILENO, &c, 1) == -1)
            {
                perror("Read Error");
                exit(EXIT_FAILURE);
            }
            if (c == '\033')
            {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) == -1 || read(STDIN_FILENO, &seq[1], 1) == -1)
                    continue;

                if (seq[0] == '[')
                {
                    if (seq[1] == 'A')
                    {
                        // Up Arrow Key Pressed
                        if (curr->prev == NULL)
                        {
                            printf("\a");
                            continue;
                        }
                        for (int i = 0; i < index; i++)
                        {
                            printf("\b \b");
                        }
                        curr = curr->prev;
                        strcpy(input, curr->cmd);
                        printf("\r$ %s", input);
                        index = strlen(input);
                        edit_index = index;
                        // printf("\r%d %d\n", index, edit_index);
                        // printf("\n[Up Arrow Detected]\n");
                    }
                    else if (seq[1] == 'B')
                    {
                        if (curr->next == NULL)
                        {
                            printf("\a");
                            continue;
                        }
                        for (int i = 0; i < index; i++)
                        {
                            printf("\b \b");
                        }
                        curr = curr->next;
                        printf("Text: %s\n", curr->cmd);
                        strcpy(input, curr->cmd);
                        printf("\r$ %s", input);
                        index = strlen(input);
                        edit_index = index;
                        // Down Arrow Key Pressed
                        // printf("\n[Down Arrow Detected]\n");
                    }
                    else if (seq[1] == 'C')
                    {
                        // Right Arrow Key Pressed'
                        if (edit_index < index)
                        {
                            printf("\033[C");
                            edit_index++;
                        }
                        else
                        {
                            printf("\a");
                        }
                        // printf("\n[Right Arrow Detected]\n");
                    }
                    else if (seq[1] == 'D')
                    {
                        // Left Arrow Key Pressed
                        if (edit_index > 0)
                        {
                            printf("\033[D");
                            edit_index--;
                        }
                        else
                        {
                            printf("\a");
                        }
                        // printf("\n[Left Arrow Detected]\n");
                    }
                    else if (seq[1] == '3')
                    {
                        char del;
                        if (read(STDIN_FILENO, &del, 1) == -1)
                        {
                            continue;
                        }
                        if (del == '~')
                        {
                            // Delete Key Pressed
                            if (edit_index < index)
                            {
                                for (int i = edit_index; i < index; i++)
                                {
                                    printf("\033[C");
                                }
                                for (int i = edit_index; i < index; i++)
                                {
                                    input[i] = input[i + 1];
                                    printf("\b \b");
                                }
                                index--;
                                input[index] = '\0';
                                strcpy(curr->cmd, input);
                                printf("\r$ %s", input);
                                for (int i = edit_index; i < index; i++)
                                {
                                    printf("\033[D");
                                }
                            }
                        }
                    }
                }
                continue;
            }
            if (c == '\n')
            {
                input[index] = '\0';
                strcpy(tail->cmd, input);
                // printf("\n%s", tail->cmd);
                printf("\n");
                double_tab = 0;
                break;
            }
            if (c == '\t')
            {
                int partial_completion = 0;
                int newLen = autocomplete(input, double_tab, &partial_completion);
                if (newLen > 0)
                {
                    for (int i = index; i > 0; i--)
                    {
                        printf("\b \b");
                    }
                    index = newLen;
                    if (partial_completion == 0)
                    {
                        input[index++] = ' ';
                        input[index] = '\0';
                    }
                    printf("%s", input);
                    strcpy(tail->cmd, input);
                    double_tab = 0;
                    // printf(" %d %d", partial_completion, index);
                }
                else if (newLen == 0)
                {

                    printf("$ %s", input);
                    double_tab = 0;
                }
                else
                {
                    printf("\a");
                    double_tab = 1;
                }
                continue;
            }
            if (c == 127)
            {
                // printf("Edit: %d\n", edit_index);
                // printf("\nBackspace: %d %d\n", index, edit_index);
                if (edit_index == index && index > 0)
                {
                    printf("\b \b");
                    index--;
                    edit_index--;
                    input[index] = '\0';
                    strcpy(tail->cmd, input);
                    continue;
                }
                if (edit_index < index)
                {
                    // printf("%d %d\n", index, edit_index);
                    for (int i = edit_index; i < index; i++)
                    {
                        printf("\033[C");
                    }
                    for (int i = edit_index; i < index; i++)
                    {
                        input[i - 1] = input[i];
                        printf("\b \b");
                        // printf("\033[D");

                        // tail->cmd[i - 1] = tail->cmd[i];
                    }
                    // printf("\b \b");
                    edit_index--;
                    index--;
                    input[index] = '\0';
                    strcpy(tail->cmd, input);
                    printf("\r$ %s", input);

                    for (int i = edit_index; i < index; i++)
                    {
                        printf("\033[D");
                    }
                    // printf("\b \b");
                }
                // tail->cmd[index] = '\0';
                double_tab = 0;
                continue;
            }

            input[edit_index] = c;
            strcpy(tail->cmd, input);
            printf("%c", c);
            double_tab = 0;
            edit_index++;
            if (edit_index > index)
            {
                // printf("Index updated");
                index++;
            }
        }
        // fgets(input, 1024, stdin);

        if (index == 0)
        {
            continue;
        }
        // input[strcspn(input, "\n")] = '\0';

        int cmd_count = tokenize_cmd(input, cmd);
        if (cmd_count == 0)
        {
            continue;
        }
        else if (cmd_count == 1)
        {
            execute_cmd(input);
        }
        else
        {
            execute_pipe_cmd(cmd, cmd_count);
        }
        // else if(cmd_count>1){
        //     printf("Multiple commands not supported\n");

        //     continue;
        // }

        // execute_cmd(input);
    }

    return 0;
}

void execute_cmd(char *input)
{

    char *out_file = NULL;
    int out_file_type = STDOUT_FILENO;
    int mode = O_WRONLY | O_CREAT | O_TRUNC;
    char *args[MAX_TOKENS];
    int token_num = tokenize(input, args);

    for (int i = 0; i < token_num; i++)
    {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "1>") == 0)
        {
            if (i + 1 < token_num)
            {
                out_file = args[i + 1];
            }
            for (int j = i; j + 2 <= token_num; j++)
            {
                args[j] = args[j + 2];
            }
            token_num -= 2;
            out_file_type = STDOUT_FILENO;
            mode = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        }
        if (strcmp(args[i], "2>") == 0)
        {
            if (i + 1 < token_num)
            {
                out_file = args[i + 1];
            }
            for (int j = i; j + 2 <= token_num; j++)
            {
                args[j] = args[j + 2];
            }
            token_num -= 2;
            out_file_type = STDERR_FILENO;
            mode = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        }
        if (strcmp(args[i], ">>") == 0 || strcmp(args[i], "1>>") == 0)
        {
            if (i + 1 < token_num)
            {
                out_file = args[i + 1];
            }
            for (int j = i; j + 2 <= token_num; j++)
            {
                args[j] = args[j + 2];
            }
            token_num -= 2;
            out_file_type = STDOUT_FILENO;
            mode = O_WRONLY | O_CREAT | O_APPEND;
            break;
        }
        if (strcmp(args[i], "2>>") == 0)
        {
            if (i + 1 < token_num)
            {
                out_file = args[i + 1];
            }
            for (int j = i; j + 2 <= token_num; j++)
            {
                args[j] = args[j + 2];
            }
            token_num -= 2;
            out_file_type = STDERR_FILENO;
            mode = O_WRONLY | O_CREAT | O_APPEND;
            break;
        }
    }
    args[token_num] = NULL;

    int saved_stdout = dup(out_file_type);
    if (saved_stdout < 0)
    {
        perror("stdout");
        exit(EXIT_FAILURE);
    }

    int found = 0;
    int check = 0;
    for (int i = 0; i < ARRAY_SIZE(builtin_commands); i++)
    {
        if (strncmp(args[0], builtin_commands[i].name,
                    strlen(builtin_commands[i].name)) == 0)
        {
            check = builtin_commands[i].process(args, token_num, out_file, out_file_type, mode);
            printf("Builtin execution complete\n");
            found = 1;
        }
    }

    if (found == 0)
    {
        char *cmd_path = find_in_path(args[0]);
        if (cmd_path != NULL)
        {
            args[token_num] = NULL;
            fork_and_execute(cmd_path, token_num, args, out_file, out_file_type, mode);
            found = 1;
        }
    }

    if (found == 0 || check != 0)
    {
        printf("%s: command not found\n", args[0]);
    }

    for (int i = 0; i < token_num; i++)
    {
        free(args[i]);
    }

    if (dup2(saved_stdout, out_file_type) < 0)
    {
        perror("dup2 restore");
        exit(EXIT_FAILURE);
    }

    close(saved_stdout);
}

void finish_token(char *buffer, int *buf_idx, char **tokens, int *token_count)
{
    if (*buf_idx > 0)
    {

        buffer[*buf_idx] = '\0';

        tokens[*token_count] = (char *)malloc(strlen(buffer) + 1);
        if (tokens[*token_count] == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        strcpy(tokens[*token_count], buffer);

        (*token_count)++;
        *buf_idx = 0;
    }
}

void execute_pipe_cmd(char **cmds, int cmd_count)
{
    int pipefd[2];
    int prev_pipe = STDIN_FILENO;
    for (int i = 0; i < cmd_count; i++)
    {
        char *args[MAX_TOKENS];
        // int token_num = tokenize(cmds[i], args);
        if (i < cmd_count - 1)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        // printf("%d\n", i);
        pid_t pid = fork();
        if (pid == 0)
        {
            if (i < cmd_count - 1)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0)
                {
                    perror("dup2 pipe write");
                    exit(EXIT_FAILURE);
                }
            }
            if (i > 0)
            {
                if (dup2(prev_pipe, STDIN_FILENO) < 0)
                {
                    perror("dup2 pipe read");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < cmd_count - 1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            execute_cmd(cmds[i]);
            exit(EXIT_SUCCESS);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if (i < cmd_count - 1)
            {
                close(pipefd[1]);
                prev_pipe = pipefd[0];
            }
        }
    }
}

int tokenize_cmd(const char *input, char *cmds[MAX_TOKENS])
{
    char *cmd;
    char *working_cp = strdup(input);
    int count = 0;
    bool double_quote = false, single_quote = false;
    for (int i = 0; i < strlen(working_cp); i++)
    {
        if (working_cp[i] == '"')
        {
            double_quote = !double_quote;
        }
        if (working_cp[i] == '\'')
        {
            single_quote = !single_quote;
        }
        if (working_cp[i] == '|' && (double_quote || single_quote))
        {
            working_cp[i] = ' ';
        }
    }

    cmd = strtok(working_cp, "|");

    while (cmd && count < MAX_TOKENS)
    {
        while (isspace((unsigned char)*cmd))
        {
            cmd++;
        }
        // printf("%s\n", cmd);
        cmds[count++] = strdup(cmd);
        cmd = strtok(NULL, "|");
    }

    cmds[count] = NULL;
    free(working_cp);

    return count;
}

int tokenize(const char *line, char **tokens)
{
    parse_state_e state = STATE_NORMAL;
    parse_state_e prev_state = STATE_NORMAL;
    char buffer[MAX_LENGTH];
    int buf_idx = 0;
    int token_count = 0;

    for (int i = 0; i < (int)strlen(line); i++)
    {
        char c = line[i];

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

int is_executable(const char *path) { return access(path, X_OK) == 0; }

char *find_in_path(const char *command)
{
    char *path_env = getenv("PATH");
    if (path_env == NULL)
        return NULL;

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    static char full_path[PATH_MAX];

    while (dir != NULL)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        if (is_executable(full_path) == 1)
        {
            free(path_copy);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

void fork_and_execute(char *cmd_path, int argc, char **args, char *out_file, int out_file_type, int mode)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        if (out_file != NULL)
        {
            int fd = open(out_file, mode, 0666);
            if (fd < 0)
            {
                perror("open redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, out_file_type);
            close(fd);
        }
        execv(cmd_path, args);
        perror("execv");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("fork");
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

int process_exit(char *args[], int argc, char *out_file, int out_file_type, int mode)
{
    if (argc == 2)
    {
        if (strncmp(args[1], "0", 1) == 0)
        {
            exit(EXIT_SUCCESS);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Incorrect use of exit\n");
        return 1;
    }
    return 0;
}

int process_echo(char *args[], int argc, char *out_file, int out_file_type, int mode)
{
    if (out_file != NULL)
    {
        int fd = open(out_file, mode, 0666);
        if (fd < 0)
        {
            perror("open redirection");
            exit(EXIT_FAILURE);
        }
        dup2(fd, out_file_type);
        close(fd);
    }
    for (int i = 1; i < argc; i++)
    {
        if (i < argc - 1)
        {
            printf("%s ", args[i]);
        }
        else
        {
            printf("%s", args[i]);
        }
    }
    printf("\n");
    return 0;
}

int process_type(char *args[], int argc, char *out_file, int out_file_type, int mode)
{
    if (argc == 2)
    {
        for (int i = 0; i < ARRAY_SIZE(builtin_commands); i++)
        {
            if (strncmp(args[1], builtin_commands[i].name,
                        strlen(builtin_commands[i].name)) == 0)
            {
                printf("%s is a shell builtin\n", builtin_commands[i].name);
                return 0;
            }
        }

        char *path = find_in_path(args[1]);
        if (path != NULL)
        {
            printf("%s is %s\n", args[1], path);
            return 0;
        }
        printf("%s: not found\n", args[1]);
    }

    return 0;
}

int process_pwd(char *args[], int argc, char *out_file, int out_file_type, int mode)
{
    char current_path[PATH_MAX];
    if (getcwd(current_path, PATH_MAX) != NULL)
    {
        printf("%s\n", current_path);
    }
    else
    {
        perror("getcwd");
    }
    return 0;
}

int process_cd(char *args[], int argc, char *out_file, int out_file_type, int mode)
{
    if (strncmp(args[1], "~", 1) == 0)
    {
        char *home_env = getenv("HOME");
        if (home_env == NULL)
            perror("HOME");
        if (chdir(home_env) != 0)
            perror("cd HOME");
    }
    else if (chdir(args[1]) != 0)
    {
        printf("cd: %s: No such file or directory\n", args[1]);
    }
    return 0;
}

void disable_raw_mode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag |= ICANON; // Enable canonical mode
    term.c_lflag |= ECHO;   // Enable echo

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void enable_raw_mode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~ICANON; // Disable canonical mode
    term.c_lflag &= ~ECHO;   // Disable echo
    term.c_cc[VMIN] = 1;     // Read one character at a time
    term.c_cc[VTIME] = 0;    // No timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
    // tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    // struct termios raw = orig_termios;
    // raw.c_lflag &= ~(ECHO | ICANON);
    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int autocomplete(char *input, int double_tab, int *partial_completion)
{

    // printf("\nIn autocomplete: %s\n", input);
    if (double_tab)
    {
        printf("\n");
    }
    int found = 0, idx = -1;
    for (int i = 0; i < ARRAY_SIZE(builtin_commands); i++)
    {
        if ((strncmp(input, builtin_commands[i].name,
                     strlen(input)) == 0))
        {
            // printf("%s\n", builtin_commands[i].name);
            found = 1;
            idx = i;
        }
    }

    if (found == 1)
    {
        strcpy(input, builtin_commands[idx].name);
        return strlen(builtin_commands[idx].name);
    }

    char *path_env = getenv("PATH");
    // printf("\n%s\n", path_env);

    if (path_env == NULL)
        return -1;
    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    char possible[1000][PATH_MAX];
    int possible_count = 0;

    while (dir != NULL)
    {
        DIR *dp = opendir(dir);
        if (dp == NULL)
        {
            dir = strtok(NULL, ":");
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dp)) != NULL)
        {
            // Check if the file name starts with input
            if (strncmp(entry->d_name, input, strlen(input)) == 0)
            {
                // Construct the full path
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);

                // Check if it's executable
                if (access(full_path, X_OK) == 0 && !is_duplicate(possible, possible_count, entry->d_name))
                {
                    strcpy(possible[possible_count++], entry->d_name);
                }
            }
        }

        closedir(dp);
        dir = strtok(NULL, ":");
    }

    if (possible_count == 0)
    {
        // printf("\a"); // Bell sound for no match
        free(path_copy);
        return -1;
    }

    if (possible_count == 1)
    {
        // printf("Partial completion: %u\n", *partial_completion);
        strcpy(input, possible[0]);
        free(path_copy);
        return strlen(possible[0]);
    }

    // Find the longest common prefix
    char lcp[PATH_MAX];
    longest_common_prefix(possible, possible_count, lcp);

    if (strlen(lcp) > strlen(input))
    {
        strcpy(input, lcp);
        free(path_copy);
        *partial_completion = 1;
        return strlen(lcp);
    }

    // On double tab, show all possibilities
    if (double_tab)
    {
        qsort(possible, possible_count, PATH_MAX, compare_strings);
        for (int i = 0; i < possible_count; i++)
        {
            printf("%s  ", possible[i]);
        }
        printf("\n");
        return 0;
    }

    free(path_copy);
    return -1;

    if (double_tab)
    {
        for (int i = 0; i < possible_count; i++)
        {
            printf("%s  ", possible[i]);
        }
        printf("\n");
        free(path_copy);
        return 0;
    }
    if (possible_count == 1)
    {
        strcpy(input, possible[0]);
        return strlen(possible[0]);
    }

    free(path_copy);
    return -1; // Command not found
}

bool is_duplicate(char possible[][PATH_MAX], int count, char *name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(possible[i], name) == 0)
        {
            return true;
        }
    }
    return false;
}

int compare_strings(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

void longest_common_prefix(char strings[][PATH_MAX], int count, char *result)
{
    if (count == 0)
    {
        result[0] = '\0';
        return;
    }

    int index = 0;
    while (1)
    {
        char current_char = strings[0][index];
        for (int i = 1; i < count; i++)
        {
            if (strings[i][index] != current_char || strings[i][index] == '\0')
            {
                result[index] = '\0';
                return;
            }
        }
        result[index] = current_char;
        index++;
    }
}