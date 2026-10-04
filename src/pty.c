#define _GNU_SOURCE
#include "ptymux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

int create_pty_session(const char *name)
{
    int index = session_create(name);

    if (index < 0)
        return -1;

    int master = posix_openpt(O_RDWR | O_NOCTTY);

    if (master < 0) {
        perror("posix_openpt");
        return -1;
    }

    if (grantpt(master) < 0 || unlockpt(master) < 0) {
        perror("PTY setup");
        close(master);
        return -1;
    }

    char *slave_name = ptsname(master);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(master);
        return -1;
    }

    if (pid == 0) {
        setsid();

        int slave = open(slave_name, O_RDWR);

        if (slave < 0)
            exit(1);

        ioctl(slave, TIOCSCTTY, 0);

        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);

        if (slave > STDERR_FILENO)
            close(slave);

        execl("/bin/bash", "bash", NULL);
        exit(1);
    }

    fcntl(master, F_SETFL, O_NONBLOCK);

    sessions[index].master_fd = master;
    sessions[index].pid = pid;

    return index;
}

void close_session(int index)
{
    if (index < 0 || index >= session_count)
        return;

    if (sessions[index].pid > 0)
        kill(sessions[index].pid, SIGHUP);

    if (sessions[index].master_fd >= 0)
        close(sessions[index].master_fd);

    sessions[index].master_fd = -1;
    sessions[index].pid = -1;
}

void reap_children(void)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}
