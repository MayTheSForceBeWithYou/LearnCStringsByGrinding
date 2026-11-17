/* lenreps.c -- s01-r01-strlen-reps: your four functions. */
#include "lenreps.h"

/* ---- Your definitions of the lenreps.h functions go below this line. ---- */

size_t len_index(const char *s)
{
    size_t i = 0;

    while (s[i] != '\0')
        i++;
    return i;
}

size_t len_diff(const char *s)
{
    const char *p = s;

    while (*p != '\0')
        p++;
    return (size_t)(p - s);
}

size_t len_bounded(const char *s, size_t maxlen)
{
    size_t n = 0;

    while (n < maxlen && s[n] != '\0')
        n++;
    return n;
}

int len_fits(const char *s, size_t cap)
{
    return len_bounded(s, cap) < cap;
}
