/* gap.c -- s02-r01-gap-reps: your four functions. */
#include "gap.h"

#include <string.h>

/* ---- Your definitions of the gap.h functions go below this line. ---- */

size_t gap_open(char *buf, size_t cap, size_t len, size_t at, size_t n)
{
    if (at > len || n > cap - len)
        return len;
    memmove(buf + at + n, buf + at, len - at);
    return len + n;
}

size_t gap_close(char *buf, size_t cap, size_t len, size_t at, size_t n)
{
    (void)cap;
    if (at > len || n > len - at)
        return len;
    memmove(buf + at, buf + at + n, len - at - n);
    return len - n;
}

size_t gap_delete(char *buf, size_t cap, size_t len, size_t from, size_t to)
{
    if (from > to || to > len)
        return len;
    return gap_close(buf, cap, len, from, to - from);
}

size_t gap_insert(char *buf, size_t cap, size_t len, size_t at, const char *src, size_t n)
{
    size_t after = gap_open(buf, cap, len, at, n);

    if (after == len && n > 0)
        return len;
    memcpy(buf + at, src, n);
    return after;
}
