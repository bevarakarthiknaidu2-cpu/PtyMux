#include "ptymux.h"
#include <string.h>

void buffer_append(Session *s, const char *data, size_t len)
{
    if (len >= BUFFER_SIZE) {
        data += len - BUFFER_SIZE;
        len = BUFFER_SIZE;
    }

    if (s->buffer_len + len > BUFFER_SIZE) {
        size_t drop = s->buffer_len + len - BUFFER_SIZE;
        memmove(s->buffer,
                s->buffer + drop,
                s->buffer_len - drop);
        s->buffer_len -= drop;
    }

    memcpy(s->buffer + s->buffer_len, data, len);
    s->buffer_len += len;
}

void buffer_clear(Session *s)
{
    s->buffer_len = 0;
}
