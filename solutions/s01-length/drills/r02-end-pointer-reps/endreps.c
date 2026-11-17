/* endreps.c -- s01-r02-end-pointer-reps: your four functions. */
#include "endreps.h"

/* ---- Your definitions of the endreps.h functions go below this line. ---- */

char *end_of(char *s)
{
    while (*s != '\0')
        s++;
    return s;
}

char *last_char(char *s)
{
    char *e = end_of(s);

    return e == s ? NULL : e - 1;
}

size_t len_between(const char *begin, const char *end)
{
    return (size_t)(end - begin);
}

size_t room_left(const char *p, const char *buf_end)
{
    size_t bytes = len_between(p, buf_end);

    return bytes > 0 ? bytes - 1 : 0;
}
