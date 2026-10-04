#include "ptymux.h"
#include <signal.h>

static volatile sig_atomic_t resized = 0;

static void handle_winch(int sig)
{
    (void)sig;
    resized = 1;
}

void setup_signals(void)
{
    signal(SIGWINCH, handle_winch);
}

int terminal_resized(void)
{
    if (resized) {
        resized = 0;
        return 1;
    }

    return 0;
}
