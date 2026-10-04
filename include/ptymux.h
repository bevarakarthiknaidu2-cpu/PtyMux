#ifndef PTYMUX_H
#define PTYMUX_H

#include <sys/types.h>
#include <stddef.h>

#define MAX_SESSIONS 8
#define BUFFER_SIZE 65536
#define SOCKET_PATH "/tmp/ptymux.sock"

typedef struct {
    int id;
    int master_fd;
    pid_t pid;
    char name[32];
    int active;
    char buffer[BUFFER_SIZE];
    size_t buffer_len;
} Session;

extern Session sessions[MAX_SESSIONS];
extern int session_count;
extern int active_session;

void print_help(void);

int create_pty_session(const char *name);
void close_session(int index);
void reap_children(void);

void session_list(void);
int session_create(const char *name);
void session_kill(int index);
void session_next(void);
void session_prev(void);
void session_cleanup(void);

int terminal_raw(void);
void terminal_restore(void);
void terminal_get_size(void);
void terminal_set_size_all(void);

void buffer_append(Session *s, const char *data, size_t len);
void buffer_clear(Session *s);

int create_server_socket(void);
int connect_server_socket(void);

void server_run(void);
void client_run(void);

void ptymux_multiplexer_ready(void);
void setup_signals(void);
int terminal_resized(void);
#endif
