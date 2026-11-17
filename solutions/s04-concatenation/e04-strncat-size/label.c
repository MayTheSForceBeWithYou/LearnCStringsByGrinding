/* label.c -- s04-e04-strncat-size, fixed: strncat's third argument is how many
 * CHARACTERS may be appended, not how big the destination is. The room for characters
 * in a buffer of `cap` bytes that already holds `have` of them is cap - have - 1: the
 * -1 is the terminator strncat always writes on top of whatever it appended. */
#include "label.h"

#include <string.h>

/* Appends at most `room` characters of src to dst. */
static void append_n(char *dst, const char *src, size_t room)
{
    strncat(dst, src, room);
}

int label_append(char *dst, size_t cap, const char *src)
{
    size_t have, want;

    if (cap == 0)
        return -1;
    have = strlen(dst);
    want = strlen(src);
    append_n(dst, src, cap - have - 1);
    return have + want < cap ? 0 : -1;
}

int label_build(char *dst, size_t cap, const char *kind, const char *text)
{
    int rc = 0;

    if (cap > 0)
        dst[0] = '\0';
    if (label_append(dst, cap, kind) != 0)
        rc = -1;
    if (label_append(dst, cap, ": ") != 0)
        rc = -1;
    if (label_append(dst, cap, text) != 0)
        rc = -1;
    return rc;
}
