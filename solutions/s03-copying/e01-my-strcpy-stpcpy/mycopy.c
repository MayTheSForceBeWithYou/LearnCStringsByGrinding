/* mycopy.c -- s03-e01-my-strcpy-stpcpy: your my_strcpy and my_stpcpy. */
#include "mycopy.h"

/* ---- Your definitions of the mycopy.h functions go below this line. ---- */

char *my_stpcpy(char *restrict dst, const char *restrict src)
{
    while ((*dst = *src) != '\0') {
        dst++;
        src++;
    }
    return dst;
}

char *my_strcpy(char *restrict dst, const char *restrict src)
{
    my_stpcpy(dst, src);
    return dst;
}
