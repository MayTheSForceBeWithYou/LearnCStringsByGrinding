/* mylen.c -- s01-e01-my-strlen: your my_strlen and my_strnlen. */
#include "mylen.h"

/* ---- Your definitions of the mylen.h functions go below this line. ---- */

size_t my_strlen(const char *s)
{
    const char *p = s;

    while (*p != '\0')
        p++;
    return (size_t)(p - s);
}

size_t my_strnlen(const char *s, size_t maxlen)
{
    size_t n = 0;

    while (n < maxlen && s[n] != '\0')
        n++;
    return n;
}
