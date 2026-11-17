/* field.c -- s03-r02-fixed-field-reps: your four field functions. */
#include "field.h"

/* ---- Your definitions of the field.h functions go below this line. ---- */

#include <string.h>

int field_put(char *field, size_t width, const char *s)
{
    size_t len = strlen(s);
    strncpy(field, s, width); /* the one job strncpy was made for */
    return len > width;
}

size_t field_len(const char *field, size_t width)
{
    return strnlen(field, width);
}

size_t field_get(char *dst, size_t cap, const char *field, size_t width)
{
    size_t len = strnlen(field, width);
    size_t n = len < cap ? len : cap - 1;
    memcpy(dst, field, n);
    dst[n] = '\0';
    return len;
}

int field_eq(const char *field, size_t width, const char *s)
{
    size_t len = strnlen(field, width);
    return strlen(s) == len && memcmp(field, s, len) == 0;
}
