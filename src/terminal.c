#define _DEFAULT_SOURCE
#include "ptymux.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios old_termios;

int terminal_raw(void)
{
    if (tcgetattr(STDIN_FILENO, &old_termios) < 0)
        return -1;

    struct termios raw = old_termios;

    cfmakeraw(&raw);

    return tcsetattr(STDIN_FILENO,
                     TCSAFLUSH,
                     &raw);
}

void terminal_restore(void)
{
    tcsetattr(STDIN_FILENO,
                TCSAFLUSH,
                &old_termios);
}

void terminal_get_size(void)
{
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
        printf("Terminal size: %d x %d\n",
               ws.ws_col,
               ws.ws_row);
}

void terminal_set_size_all(void)
{
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
        return;

    for (int i = 0; i < session_count; i++) {
        if (sessions[i].master_fd >= 0)
            ioctl(sessions[i].master_fd,
                  TIOCSWINSZ,
                  &ws);
    }
}
