/* field.c -- s01-e02-fixed-fields: your three functions. */
#include "field.h"

#include <string.h>

/* ---- Your definitions of the field.h functions go below this line. ---- */

size_t field_len(const char *field, size_t width)
{
    return strnlen(field, width);
}

int field_eq(const char *a, const char *b, size_t width)
{
    size_t la = strnlen(a, width), lb = strnlen(b, width);

    return la == lb && memcmp(a, b, la) == 0;
}

size_t field_to_cstr(char *dst, size_t cap, const char *field, size_t width)
{
    size_t len = strnlen(field, width);

    if (cap > 0) {
        size_t keep = len < cap - 1 ? len : cap - 1;
        memcpy(dst, field, keep);
        dst[keep] = '\0';
    }
    return len;
}
