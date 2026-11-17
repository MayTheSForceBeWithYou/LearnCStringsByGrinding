/* mystrlcpy.c -- s03-e03-my-strlcpy: your my_strlcpy. */
#include "mystrlcpy.h"

/* ---- Your definitions of the mystrlcpy.h functions go below this line. ---- */

size_t my_strlcpy(char *restrict dst, const char *restrict src, size_t size)
{
    size_t i = 0;
    for (; src[i] != '\0'; i++) {
        if (i + 1 < size)
            dst[i] = src[i];
    }
    if (size > 0)
        dst[i < size ? i : size - 1] = '\0';
    return i;
}
