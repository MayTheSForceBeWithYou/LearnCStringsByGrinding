/* walk.c -- s00-r01-walk-reps: your five functions. */
#include "walk.h"

/* ---- Your definitions of the walk.h functions go below this line. ---- */

size_t walk_len(const char *s)
{
    const char *p = s;

    while (*p != '\0')
        p++;
    return (size_t)(p - s);
}

const char *walk_end(const char *s)
{
    while (*s != '\0')
        s++;
    return s;
}

size_t walk_count(const char *s, char c)
{
    size_t n = 0;

    if (c == '\0')
        return 0;
    for (const char *p = s; *p != '\0'; p++) {
        if (*p == c)
            n++;
    }
    return n;
}

int walk_last(const char *s)
{
    const char *p = s;

    if (*p == '\0')
        return -1;
    while (p[1] != '\0')
        p++;
    return (unsigned char)*p;
}

int walk_is_empty(const char *s)
{
    return *s == '\0';
}
