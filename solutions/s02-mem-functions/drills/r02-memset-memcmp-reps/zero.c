/* zero.c -- s02-r02-memset-memcmp-reps: your four functions. */
#include "zero.h"

#include <string.h>

/* ---- Your definitions of the zero.h functions go below this line. ---- */

void zero_array(void *base, size_t n, size_t size)
{
    memset(base, 0, n * size);
}

int ranges_equal(const void *a, const void *b, size_t n)
{
    return memcmp(a, b, n) == 0;
}

size_t common_prefix(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = a, *pb = b;
    size_t i = 0;

    while (i < n && pa[i] == pb[i])
        i++;
    return i;
}

int all_equal_to(const void *p, int c, size_t n)
{
    const unsigned char *q = p;
    unsigned char b = (unsigned char)c;

    if (n == 0)
        return 1;
    return q[0] == b && memcmp(q, q + 1, n - 1) == 0;
}
