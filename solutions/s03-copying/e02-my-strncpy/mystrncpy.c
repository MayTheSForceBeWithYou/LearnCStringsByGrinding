/* mystrncpy.c -- s03-e02-my-strncpy: your my_strncpy and my_stpncpy. */
#include "mystrncpy.h"

/* ---- Your definitions of the mystrncpy.h functions go below this line. ---- */

char *my_stpncpy(char *restrict dst, const char *restrict src, size_t n)
{
    size_t i = 0;
    while (i < n && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    char *end = dst + i; /* the first NUL, or dst + n if there is no room for one */
    while (i < n)
        dst[i++] = '\0';
    return end;
}

char *my_strncpy(char *restrict dst, const char *restrict src, size_t n)
{
    my_stpncpy(dst, src, n);
    return dst;
}
