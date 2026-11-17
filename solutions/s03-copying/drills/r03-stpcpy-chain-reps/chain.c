/* chain.c -- s03-r03-stpcpy-chain-reps: your four chain functions. */
#include "chain.h"

/* ---- Your definitions of the chain.h functions go below this line. ---- */

#include <string.h>

char *chain_two(char *dst, const char *a, const char *b)
{
    return stpcpy(stpcpy(dst, a), b);
}

size_t chain_path(char *dst, const char *a, const char *b, const char *c)
{
    char *p = stpcpy(dst, a);
    *p++ = '/';
    p = stpcpy(p, b);
    *p++ = '/';
    p = stpcpy(p, c);
    return (size_t)(p - dst);
}

char *chain_repeat(char *dst, const char *s, size_t n)
{
    char *p = dst;
    *p = '\0';
    while (n-- > 0)
        p = stpcpy(p, s);
    return p;
}

char *chain_bounded(char *dst, size_t cap, const char *const *parts, size_t count)
{
    if (cap == 0)
        return NULL;
    char *p = dst;
    size_t room = cap - 1; /* characters that still fit, keeping one byte for the terminator */
    *p = '\0';
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(parts[i]);
        if (len > room) {
            dst[0] = '\0';
            return NULL;
        }
        memcpy(p, parts[i], len + 1);
        p += len;
        room -= len;
    }
    return p;
}
