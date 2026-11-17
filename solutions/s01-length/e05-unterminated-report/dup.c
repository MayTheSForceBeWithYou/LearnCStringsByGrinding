/* dup.c -- s01-e05-unterminated-report, fixed: the allocation is one byte larger than
 * the text, and that byte is written. The size is computed once, by the function whose
 * job it is, so the two cannot go out of step. */
#include "dup.h"

#include <stdlib.h>
#include <string.h>

size_t field_dup_size(const char *field, size_t width)
{
    return strnlen(field, width) + 1;
}

char *field_dup(const char *field, size_t width)
{
    size_t len = strnlen(field, width);
    char *out = malloc(len + 1);

    if (out == NULL)
        return NULL;
    memcpy(out, field, len);
    out[len] = '\0';
    return out;
}
