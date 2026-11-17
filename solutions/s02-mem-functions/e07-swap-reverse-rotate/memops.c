/* memops.c -- s02-e07-swap-reverse-rotate: your three functions. */
#include "memops.h"

#include <string.h>

/* ---- Your definitions of the memops.h functions go below this line. ---- */

void mem_swap(void *a, void *b, size_t size)
{
    unsigned char *pa = a, *pb = b;
    char tmp[MEMOPS_CHUNK];

    if (pa == pb)
        return;
    while (size > 0) {
        size_t chunk = size < sizeof tmp ? size : sizeof tmp;

        memcpy(tmp, pa, chunk);
        memcpy(pa, pb, chunk);
        memcpy(pb, tmp, chunk);
        pa += chunk;
        pb += chunk;
        size -= chunk;
    }
}

void mem_reverse(void *base, size_t n, size_t size)
{
    unsigned char *p = base;

    for (size_t i = 0; i + 1 < n; i++, n--)
        mem_swap(p + i * size, p + (n - 1) * size, size);
}

void mem_rotate_left(void *base, size_t n, size_t size, size_t k)
{
    if (n == 0)
        return;
    k %= n;
    if (k == 0)
        return;
    mem_reverse(base, k, size);
    mem_reverse((unsigned char *)base + k * size, n - k, size);
    mem_reverse(base, n, size);
}
