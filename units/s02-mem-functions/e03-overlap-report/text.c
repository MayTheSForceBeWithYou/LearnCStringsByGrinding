/* text.c -- s02-e03-overlap-report. One line here is wrong. Do not change anything
 * until you have answered the questions in answers.txt. */
#include "text.h"

#include <string.h>

size_t text_move_count(size_t len, size_t at, size_t n)
{
    return len - at - n + 1; /* the tail, and the terminator */
}

size_t text_delete(char *buf, size_t len, size_t at, size_t n)
{
    size_t move = text_move_count(len, at, n);

    memcpy(buf + at, buf + at + n, move);
    return len - n;
}
