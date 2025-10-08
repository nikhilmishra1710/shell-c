#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void disable_raw_mode(void);
void enable_raw_mode(void);

#endif