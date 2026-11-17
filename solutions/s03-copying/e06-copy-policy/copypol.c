/* copypol.c -- s03-e06-copy-policy: your three copies. Write the contracts in
 * copypol.h first. */
#include "copypol.h"

/* ---- Your definitions of the copypol.h functions go below this line. ---- */

#include <string.h>

int copy_trunc(char *dst, size_t cap, const char *src)
{
    if (cap == 0)
        return COPY_NO_ROOM;
    size_t len = strlen(src);
    size_t n = len < cap ? len : cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
    return len < cap ? COPY_OK : COPY_TRUNCATED;
}

int copy_strict(char *dst, size_t cap, const char *src)
{
    if (cap == 0)
        return COPY_NO_ROOM;
    size_t len = strlen(src);
    if (len >= cap) {
        dst[0] = '\0';
        return COPY_NO_ROOM;
    }
    memcpy(dst, src, len + 1);
    return COPY_OK;
}

int copy_n(char *dst, size_t cap, const char *src, size_t n)
{
    if (cap == 0)
        return COPY_NO_ROOM;
    if (n >= cap) { /* n + 1 > cap, written so that n == SIZE_MAX cannot wrap */
        dst[0] = '\0';
        return COPY_NO_ROOM;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
    return COPY_OK;
}
