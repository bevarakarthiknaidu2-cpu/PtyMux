#include "ptymux.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: ./ptymux server|client|help\n");
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        server_run();
    }
    else if (strcmp(argv[1], "client") == 0) {
        client_run();
    }
    else if (strcmp(argv[1], "help") == 0) {
        print_help();
    }
    else {
        printf("Unknown command\n");
        return 1;
    }

    return 0;
}
