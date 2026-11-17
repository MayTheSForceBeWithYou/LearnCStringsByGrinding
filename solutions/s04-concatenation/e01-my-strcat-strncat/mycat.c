/* mycat.c -- s04-e01-my-strcat-strncat: your my_strcat and my_strncat. */
#include "mycat.h"

/* ---- Your definitions of the mycat.h functions go below this line. ---- */

char *my_strcat(char *restrict dst, const char *restrict src)
{
    char *p = dst;

    while (*p != '\0')
        p++;
    while ((*p = *src) != '\0') {
        p++;
        src++;
    }
    return dst;
}

char *my_strncat(char *restrict dst, const char *restrict src, size_t n)
{
    char *p = dst;

    while (*p != '\0')
        p++;
    for (size_t i = 0; i < n && src[i] != '\0'; i++)
        *p++ = src[i];
    *p = '\0';
    return dst;
}
