#include "ptymux.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define PREFIX 2

static void send_command(int fd, char command)
{
    char data[2];

    data[0] = PREFIX;
    data[1] = command;

    write(fd, data, 2);
}

void client_run(void)
{
    int fd = connect_server_socket();

    if (fd < 0) {
        printf("Could not connect to PtyMux server.\n");
        printf("Start the server first:\n");
        printf("./ptymux server\n");
        return;
    }

    if (terminal_raw() < 0) {
        close(fd);
        return;
    }

    printf("\033[2J\033[H");
    printf("\033[1;36mPtyMux Client\033[0m\r\n");
    printf("Ctrl-B ? for help\r\n\r\n");

    int prefix = 0;
    int running = 1;

    while (running) {

        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(fd, &readfds);

        int maxfd = fd;

        if (STDIN_FILENO > maxfd)
            maxfd = STDIN_FILENO;

        if (select(maxfd + 1,
                   &readfds,
                   NULL,
                   NULL,
                   NULL) < 0) {
            continue;
        }

        /* Keyboard */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {

            char buf[256];

            ssize_t n = read(STDIN_FILENO,
                             buf,
                             sizeof(buf));

            if (n <= 0)
                break;

            for (ssize_t i = 0; i < n; i++) {

                unsigned char c = buf[i];

                if (prefix) {

                    prefix = 0;

                    switch (c) {

                    case 'c':
                        send_command(fd, 'c');
                        break;

                    case 'n':
                        send_command(fd, 'n');
                        break;

                    case 'p':
                        send_command(fd, 'p');
                        break;

                    case 'l':
                        send_command(fd, 'l');
                        break;

                    case 'k':
                        send_command(fd, 'k');
                        break;

                    case 'r':
                        send_command(fd, 'r');
                        break;

                    case 'd':
                        send_command(fd, 'd');
                        running = 0;
                        break;

                    case 'q':
                        send_command(fd, 'q');
                        running = 0;
                        break;

                    case '?':
                        send_command(fd, '?');
                        break;

                    case PREFIX:
                        write(fd, &c, 1);
                        break;

                    default:
                        write(fd, &c, 1);
                        break;
                    }

                }
                else if (c == PREFIX) {

                    prefix = 1;

                }
                else {

                    write(fd, &c, 1);
                }
            }
        }

        /* Server output */
        if (FD_ISSET(fd, &readfds)) {

            char buf[4096];

            ssize_t n = read(fd,
                             buf,
                             sizeof(buf));

            if (n <= 0)
                break;

            write(STDOUT_FILENO,
                  buf,
                  (size_t)n);
        }
    }

    terminal_restore();
    close(fd);

    printf("\n[PtyMux] Client detached.\n");
}
