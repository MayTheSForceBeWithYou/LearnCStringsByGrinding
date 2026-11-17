/* count.c -- s00-e02-count-and-terminate: your four functions. */
#include "count.h"

/* ---- Your definitions of the count.h functions go below this line. ---- */

size_t str_len(const char *s)
{
    size_t i = 0;

    while (s[i] != '\0')
        i++;
    return i;
}

const char *str_end(const char *s)
{
    while (*s != '\0')
        s++;
    return s;
}

int buf_is_terminated(const char *buf, size_t cap)
{
    for (size_t i = 0; i < cap; i++) {
        if (buf[i] == '\0')
            return 1;
    }
    return 0;
}

size_t buf_set(char *buf, size_t cap, char c, size_t n)
{
    size_t i;

    if (cap == 0)
        return 0;
    for (i = 0; i < n && i + 1 < cap; i++)
        buf[i] = c;
    buf[i] = '\0';
    return i;
}
