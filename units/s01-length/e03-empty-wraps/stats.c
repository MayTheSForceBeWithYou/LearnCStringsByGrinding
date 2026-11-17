/* stats.c -- s01-e03-empty-wraps. There is one defect in this file. */
#include "stats.h"

#include <string.h>

size_t count_upper(const char *s)
{
    size_t n = 0;

    for (size_t i = 0; i <= strlen(s) - 1; i++) {
        if (s[i] >= 'A' && s[i] <= 'Z')
            n++;
    }
    return n;
}

size_t count_digits(const char *s)
{
    size_t n = 0;

    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] >= '0' && s[i] <= '9')
            n++;
    }
    return n;
}
