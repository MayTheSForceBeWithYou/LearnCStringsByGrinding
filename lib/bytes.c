/* bytes.c -- see bytes.h. Plain C17. */
#include "bytes.h"

#include <string.h>

static size_t put(char *out, size_t cap, size_t at, const char *s)
{
    for (; *s != '\0'; s++, at++) {
        if (at + 1 < cap)
            out[at] = *s;
    }
    return at;
}

size_t bytes_notation(const void *p, size_t n, char *out, size_t cap)
{
    static const char hex[] = "0123456789ABCDEF";
    const unsigned char *b = p;
    size_t at = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = b[i];
        char one[5];
        if (c == '\0')
            strcpy(one, "\\0");
        else if (c == '\n')
            strcpy(one, "\\n");
        else if (c == '\t')
            strcpy(one, "\\t");
        else if (c == '\r')
            strcpy(one, "\\r");
        else if (c == '\\')
            strcpy(one, "\\\\");
        else if (c >= 0x20 && c <= 0x7E) {
            one[0] = (char)c;
            one[1] = '\0';
        } else {
            one[0] = '\\';
            one[1] = 'x';
            one[2] = hex[c >> 4];
            one[3] = hex[c & 0x0F];
            one[4] = '\0';
        }
        at = put(out, cap, at, one);
    }
    if (cap > 0)
        out[at < cap ? at : cap - 1] = '\0';
    return at;
}

const char *bytes_show(const void *p, size_t n)
{
    static char bufs[4][1024];
    static unsigned turn;
    char *out = bufs[turn++ % 4];
    if (bytes_notation(p, n, out, sizeof bufs[0]) >= sizeof bufs[0])
        memcpy(out + sizeof bufs[0] - 4, "...", 4);
    return out;
}
