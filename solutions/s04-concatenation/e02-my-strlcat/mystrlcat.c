/* mystrlcat.c -- s04-e02-my-strlcat: your my_strlcat. */
#include "mystrlcat.h"

/* ---- Your definition of the mystrlcat.h function goes below this line. ---- */

size_t my_strlcat(char *dst, const char *src, size_t size)
{
    size_t dlen = 0, slen = 0;

    while (dlen < size && dst[dlen] != '\0')
        dlen++;
    while (src[slen] != '\0')
        slen++;
    if (dlen == size)
        return size + slen; /* no terminator in dst: nothing is appended */
    for (size_t i = 0; i < slen && dlen + i + 1 < size; i++)
        dst[dlen + i] = src[i];
    {
        size_t room = size - dlen - 1;
        size_t keep = slen < room ? slen : room;

        dst[dlen + keep] = '\0';
    }
    return dlen + slen;
}
