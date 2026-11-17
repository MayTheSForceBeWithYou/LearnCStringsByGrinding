/* name.c -- s01-e06-narrow-length, fixed: a name is rejected when it is too long for
 * the length field, before anything is written. The test is on the length the field
 * can describe, NAME_MAX, not on the size of the buffer. */
#include "name.h"

#include <string.h>

int name_set(Name *n, const char *s)
{
    size_t len = strlen(s);

    if (len > NAME_MAX)
        return -1;
    memcpy(n->buf, s, len + 1);
    n->len = (unsigned char)len;
    return 0;
}

size_t name_len(const Name *n)
{
    return n->len;
}

size_t name_get(const Name *n, char *dst, size_t cap)
{
    size_t len = n->len;

    if (cap > 0) {
        size_t keep = len < cap - 1 ? len : cap - 1;
        memcpy(dst, n->buf, keep);
        dst[keep] = '\0';
    }
    return len;
}
