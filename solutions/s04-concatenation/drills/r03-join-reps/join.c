/* join.c -- s04-r03-join-reps: your three functions. */
#include "join.h"

#include <string.h>

/* ---- Your definitions of the join.h functions go below this line. ---- */

size_t join_len(const char *const *parts, size_t n, const char *sep)
{
    size_t total = 0, slen = strlen(sep);

    for (size_t i = 0; i < n; i++) {
        if (i > 0)
            total += slen;
        total += strlen(parts[i]);
    }
    return total;
}

/* Appends as much of s as fits at *at, snprintf-style. */
static void put(char *dst, size_t cap, size_t *at, const char *s, size_t slen)
{
    if (cap > 0 && *at < cap - 1) {
        size_t room = cap - 1 - *at;
        size_t keep = slen < room ? slen : room;

        memcpy(dst + *at, s, keep);
        dst[*at + keep] = '\0';
    }
    *at += slen;
}

size_t join_into(char *dst, size_t cap, const char *const *parts, size_t n, const char *sep)
{
    size_t at = 0, slen = strlen(sep);

    if (cap > 0)
        dst[0] = '\0';
    for (size_t i = 0; i < n; i++) {
        if (i > 0)
            put(dst, cap, &at, sep, slen);
        put(dst, cap, &at, parts[i], strlen(parts[i]));
    }
    return at;
}

size_t join_prefix(char *dst, size_t cap, const char *const *parts, size_t n, const char *sep,
                   size_t *fitted)
{
    size_t at = 0, slen = strlen(sep), k = 0;

    if (cap > 0)
        dst[0] = '\0';
    for (size_t i = 0; i < n; i++) {
        size_t add = strlen(parts[i]) + (i > 0 ? slen : 0);

        if (cap == 0 || add > cap - 1 - at)
            break;
        if (i > 0)
            put(dst, cap, &at, sep, slen);
        put(dst, cap, &at, parts[i], strlen(parts[i]));
        k++;
    }
    if (fitted != NULL)
        *fitted = k;
    return at;
}
