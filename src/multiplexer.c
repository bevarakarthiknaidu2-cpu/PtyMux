#include "ptymux.h"

void ptymux_multiplexer_ready(void)
{
    /*
     * PtyMux uses select() in server.c
     * to multiplex:
     *
     * 1. Client input
     * 2. PTY output
     * 3. Server connections
     */
}
