/* dup.c -- s01-e05-unterminated-report. One of these two functions is wrong. Do not
 * change anything until you have answered the questions in answers.txt. */
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
    char *out = malloc(len);

    if (out == NULL)
        return NULL;
    memcpy(out, field, len);
    return out;
}
