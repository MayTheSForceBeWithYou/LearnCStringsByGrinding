/* mymove.c -- s02-e01-my-memcpy-memmove: your my_memcpy and my_memmove. */
#include "mymove.h"

/* ---- Your definitions of the mymove.h functions go below this line. ---- */

void *my_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}

void *my_memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else if (d > s) {
        for (size_t i = n; i-- > 0;)
            d[i] = s[i];
    }
    return dst;
}
