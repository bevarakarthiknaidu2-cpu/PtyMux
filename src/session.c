#include "ptymux.h"
#include <stdio.h>
#include <string.h>

Session sessions[MAX_SESSIONS];
int session_count = 0;
int active_session = 0;

int session_create(const char *name)
{
    if (session_count >= MAX_SESSIONS)
        return -1;

    int i = session_count;

    sessions[i].id = i + 1;
    sessions[i].master_fd = -1;
    sessions[i].pid = -1;
    sessions[i].active = 1;
    sessions[i].buffer_len = 0;

    snprintf(sessions[i].name,
             sizeof(sessions[i].name),
             "%s", name);

    session_count++;

    return i;
}

void session_list(void)
{
    printf("\nSessions:\n");

    for (int i = 0; i < session_count; i++) {
        printf("%c [%d] %s\n",
               i == active_session ? '*' : ' ',
               sessions[i].id,
               sessions[i].name);
    }

    printf("\n");
}

void session_next(void)
{
    if (session_count > 0)
        active_session =
            (active_session + 1) % session_count;
}

void session_prev(void)
{
    if (session_count > 0)
        active_session =
            (active_session + session_count - 1)
            % session_count;
}

void session_kill(int index)
{
    if (index < 0 || index >= session_count)
        return;

    close_session(index);

    for (int i = index; i < session_count - 1; i++)
        sessions[i] = sessions[i + 1];

    session_count--;

    if (session_count == 0)
        active_session = 0;
    else if (active_session >= session_count)
        active_session = session_count - 1;
}

void session_cleanup(void)
{
    for (int i = 0; i < session_count; i++)
        close_session(i);

    session_count = 0;
    active_session = 0;
}
