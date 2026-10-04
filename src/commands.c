#include "ptymux.h"
#include <stdio.h>

void print_help(void)
{
    printf("\n========== PtyMux Keymap ==========\n");
    printf("Ctrl-B c       Create session\n");
    printf("Ctrl-B n       Next session\n");
    printf("Ctrl-B p       Previous session\n");
    printf("Ctrl-B l       List sessions\n");
    printf("Ctrl-B k       Kill active session\n");
    printf("Ctrl-B r       Repaint / scrollback\n");
    printf("Ctrl-B d       Detach client\n");
    printf("Ctrl-B ?       Help\n");
    printf("Ctrl-B q       Quit client\n");
    printf("Ctrl-B Ctrl-B  Send Ctrl-B to shell\n");
    printf("===================================\n\n");
}
