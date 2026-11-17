/* record.c -- s05-e03-overread-report. Do not change anything until you have answered
 * the questions in answers.txt. */
#include "record.h"

#include <stdlib.h>
#include <string.h>

static void field_copy(char *dst, const char *src, size_t n)
{
    memcpy(dst, src, n);
}

void record_set(Record *r, const char *code, int quantity)
{
    field_copy(r->code, code, CODE_WIDTH);
    r->quantity = quantity;
}

char *record_code_dup(const Record *r)
{
    char *out = malloc(CODE_WIDTH);

    if (out == NULL)
        return NULL;
    field_copy(out, r->code, CODE_WIDTH);
    return out;
}
