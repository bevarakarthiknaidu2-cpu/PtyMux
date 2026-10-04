#define _GNU_SOURCE

#include "ptymux.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>

#define BUF_SIZE 4096
#define PREFIX 2

static int server_fd = -1;
static int client_fd = -1;

static void send_text(const char *text)
{
    if (client_fd >= 0)
        (void)write(client_fd, text, strlen(text));
}

static void send_sessions(void)
{
    char msg[1024];

    int len = snprintf(msg, sizeof(msg),
                       "\r\n--- PtyMux Sessions ---\r\n");

    for (int i = 0; i < session_count; i++) {

        len += snprintf(msg + len,
                        sizeof(msg) - (size_t)len,
                        "%c [%d] %s\r\n",
                        i == active_session ? '*' : ' ',
                        sessions[i].id,
                        sessions[i].name);
    }

    snprintf(msg + len,
             sizeof(msg) - (size_t)len,
             "------------------------\r\n");

    send_text(msg);
}

static void create_new_session(void)
{
    char name[32];

    snprintf(name, sizeof(name),
             "session-%d",
             session_count + 1);

    if (create_pty_session(name) >= 0) {

        active_session = session_count - 1;

        send_text("\r\n[PtyMux] New session created.\r\n");
        send_sessions();

    } else {

        send_text("\r\n[PtyMux] Failed to create session.\r\n");
    }
}

static void kill_active(void)
{
    if (session_count == 0) {
        send_text("\r\n[PtyMux] No sessions.\r\n");
        return;
    }

    session_kill(active_session);

    if (session_count > 0) {
        if (active_session >= session_count)
            active_session = session_count - 1;
    }

    send_text("\r\n[PtyMux] Active session killed.\r\n");
    send_sessions();
}

static void repaint(void)
{
    if (session_count == 0)
        return;

    send_text("\033[2J\033[H");

    if (sessions[active_session].buffer_len > 0) {

        (void)write(client_fd,
                     sessions[active_session].buffer,
                     sessions[active_session].buffer_len);
    }
}

static void handle_command(char command)
{
    switch (command) {

    case 'c':
        create_new_session();
        break;

    case 'n':
        session_next();
        send_sessions();
        break;

    case 'p':
        session_prev();
        send_sessions();
        break;

    case 'l':
        send_sessions();
        break;

    case 'k':
        kill_active();
        break;

    case 'r':
        repaint();
        break;

    case '?':
        print_help();
        break;

    case 'd':
        send_text("\r\n[PtyMux] Detached.\r\n");
        close(client_fd);
        client_fd = -1;
        break;

    case 'q':
        send_text("\r\n[PtyMux] Client closed.\r\n");
        close(client_fd);
        client_fd = -1;
        break;

    default:
        break;
    }
}

void server_run(void)
{
    server_fd = create_server_socket();

    if (server_fd < 0)
        return;

    printf("\n=================================\n");
    printf("        PtyMux Server\n");
    printf("=================================\n");
    printf("Socket : %s\n", SOCKET_PATH);
    printf("Status : Running\n");
    printf("Waiting for client...\n\n");

    create_pty_session("session-1");

    while (1) {

        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(server_fd, &readfds);

        int maxfd = server_fd;

        if (client_fd >= 0) {

            FD_SET(client_fd, &readfds);

            if (client_fd > maxfd)
                maxfd = client_fd;
        }

        for (int i = 0; i < session_count; i++) {

            if (sessions[i].master_fd >= 0) {

                FD_SET(sessions[i].master_fd,
                       &readfds);

                if (sessions[i].master_fd > maxfd)
                    maxfd = sessions[i].master_fd;
            }
        }

        if (select(maxfd + 1,
                   &readfds,
                   NULL,
                   NULL,
                   NULL) < 0) {

            continue;
        }

        /* New client */
        if (FD_ISSET(server_fd, &readfds)) {

            int fd = accept(server_fd, NULL, NULL);

            if (fd >= 0) {

                if (client_fd >= 0)
                    close(client_fd);

                client_fd = fd;

                send_text("\033[2J\033[H");
                send_text("\033[1;36mPtyMux Client\033[0m\r\n");
                send_text("Ctrl-B ? for help\r\n");

                send_sessions();
            }
        }

        /* Client input */
        if (client_fd >= 0 &&
            FD_ISSET(client_fd, &readfds)) {

            unsigned char buf[256];

            ssize_t n = read(client_fd,
                             buf,
                             sizeof(buf));

            if (n <= 0) {

                close(client_fd);
                client_fd = -1;

            } else {

                for (ssize_t i = 0; i < n; i++) {

                    unsigned char c = buf[i];

                    /*
                     * Command packet:
                     * Ctrl-B + command
                     */
                    if (c == PREFIX && i + 1 < n) {

                        unsigned char command =
                            buf[++i];

                        if (command == PREFIX) {

                            if (session_count > 0)
                                (void)write(
                                    sessions[active_session].master_fd,
                                    &command,
                                    1);

                        } else {

                            handle_command(command);
                        }

                    } else {

                        if (session_count > 0 &&
                            sessions[active_session].master_fd >= 0) {

                            (void)write(
                                sessions[active_session].master_fd,
                                &c,
                                1);
                        }
                    }
                }
            }
        }

        /* PTY output */
        for (int i = 0; i < session_count; i++) {

            int fd = sessions[i].master_fd;

            if (fd < 0)
                continue;

            if (FD_ISSET(fd, &readfds)) {

                char buf[BUF_SIZE];

                ssize_t n = read(fd,
                                 buf,
                                 sizeof(buf));

                if (n > 0) {

                    buffer_append(
                        &sessions[i],
                        buf,
                        (size_t)n);

                    if (client_fd >= 0 &&
                        i == active_session) {

                        (void)write(
                            client_fd,
                            buf,
                            (size_t)n);
                    }
                }
            }
        }

        reap_children();
    }
}
