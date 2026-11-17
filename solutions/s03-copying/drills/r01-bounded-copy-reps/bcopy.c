/* bcopy.c -- s03-r01-bounded-copy-reps: your four copies. */
#include "bcopy.h"

/* ---- Your definitions of the bcopy.h functions go below this line. ---- */

#include <stdio.h>
#include <string.h>

size_t bc_loop(char *dst, size_t cap, const char *src)
{
    size_t i = 0;
    for (; src[i] != '\0'; i++) {
        if (i + 1 < cap)
            dst[i] = src[i];
    }
    dst[i < cap ? i : cap - 1] = '\0';
    return i;
}

int bc_format(char *dst, size_t cap, const char *src)
{
    int n = snprintf(dst, cap, "%s", src);
    return n < 0 || (size_t)n >= cap;
}

int bc_measured(char *dst, size_t cap, const char *src)
{
    size_t len = strlen(src);
    size_t n = len < cap ? len : cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
    return len >= cap;
}

int bc_strict(char *dst, size_t cap, const char *src)
{
    size_t len = strlen(src);
    if (len >= cap) {
        dst[0] = '\0';
        return -1;
    }
    memcpy(dst, src, len + 1);
    return 0;
}
