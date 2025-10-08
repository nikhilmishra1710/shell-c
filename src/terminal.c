#include "include/terminal.h"

struct termios orig_termios;

void disable_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag |= ICANON; // Enable canonical mode
    term.c_lflag |= ECHO;   // Enable echo

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void enable_raw_mode(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(tcflag_t)ICANON; // Disable canonical mode
    term.c_lflag &= ~(tcflag_t)ECHO;   // Disable echo
    term.c_cc[VMIN] = 1;               // Read one char at a time
    term.c_cc[VTIME] = 0;              // No timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
    atexit(disable_raw_mode);
}
