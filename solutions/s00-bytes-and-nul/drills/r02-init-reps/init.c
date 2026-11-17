/* init.c -- s00-r02-init-reps: your four functions. */
#include "init.h"

/* ---- Your definitions of the init.h functions go below this line. ---- */

int init_empty(char *buf, size_t cap)
{
    if (cap == 0)
        return -1;
    buf[0] = '\0';
    return 0;
}

size_t init_repeat(char *buf, size_t cap, char c, size_t n)
{
    size_t i;

    if (cap == 0)
        return 0;
    for (i = 0; i < n && i + 1 < cap; i++)
        buf[i] = c;
    buf[i] = '\0';
    return i;
}

size_t init_literal(char *buf, size_t cap, const char *lit)
{
    size_t i;

    if (cap == 0)
        return 0;
    for (i = 0; lit[i] != '\0' && i + 1 < cap; i++)
        buf[i] = lit[i];
    buf[i] = '\0';
    return i;
}

int init_terminated(const char *buf, size_t cap)
{
    for (size_t i = 0; i < cap; i++) {
        if (buf[i] == '\0')
            return 1;
    }
    return 0;
}
