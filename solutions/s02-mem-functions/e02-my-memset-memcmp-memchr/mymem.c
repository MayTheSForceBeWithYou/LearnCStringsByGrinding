/* mymem.c -- s02-e02-my-memset-memcmp-memchr: your three functions. */
#include "mymem.h"

/* ---- Your definitions of the mymem.h functions go below this line. ---- */

void *my_memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    unsigned char b = (unsigned char)c;

    for (size_t i = 0; i < n; i++)
        p[i] = b;
    return s;
}

int my_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = a, *pb = b;

    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i])
            return pa[i] < pb[i] ? -1 : 1;
    }
    return 0;
}

void *my_memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    unsigned char b = (unsigned char)c;

    for (size_t i = 0; i < n; i++) {
        if (p[i] == b)
            return (void *)(p + i);
    }
    return NULL;
}
