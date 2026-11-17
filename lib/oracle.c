/* oracle.c -- see oracle.h. Plain C17. */
#include "oracle.h"

#include <string.h>

static uint32_t state = 1;
static uint32_t cases;

void orc_seed(uint32_t seed)
{
    state = seed;
    cases = 0;
}

uint32_t orc_next(void)
{
    /* Numerical Recipes LCG; the high bits are the good ones. */
    state = state * UINT32_C(1664525) + UINT32_C(1013904223);
    return state;
}

uint32_t orc_below(uint32_t n)
{
    return (orc_next() >> 8) % n;
}

size_t orc_string(char *buf, size_t maxlen, const char *alphabet)
{
    size_t na = strlen(alphabet);
    size_t len;
    uint32_t pick = orc_below(8);
    if (pick == 0)
        len = 0;
    else if (pick == 1)
        len = maxlen;
    else
        len = orc_below((uint32_t)maxlen + 1);
    for (size_t i = 0; i < len; i++)
        buf[i] = alphabet[orc_below((uint32_t)na)];
    buf[len] = '\0';
    cases++;
    return len;
}

uint32_t orc_case(void)
{
    return cases;
}
