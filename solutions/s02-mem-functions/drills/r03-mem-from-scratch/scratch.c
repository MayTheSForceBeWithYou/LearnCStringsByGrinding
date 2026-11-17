/* scratch.c -- s02-r03-mem-from-scratch: your five functions. */
#include "scratch.h"

/* ---- Your definitions of the scratch.h functions go below this line. ---- */

void *s_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}

void *s_memmove(void *dst, const void *src, size_t n)
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

void *s_memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    unsigned char b = (unsigned char)c;

    for (size_t i = 0; i < n; i++)
        p[i] = b;
    return s;
}

int s_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = a, *pb = b;

    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i])
            return pa[i] < pb[i] ? -1 : 1;
    }
    return 0;
}

void *s_memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    unsigned char b = (unsigned char)c;

    for (size_t i = 0; i < n; i++) {
        if (p[i] == b)
            return (void *)(p + i);
    }
    return NULL;
}
