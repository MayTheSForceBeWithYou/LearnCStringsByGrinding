/* badge.c -- s05-e01-stack-overflow-report. Do not change anything until you have
 * answered the questions in answers.txt. */
#include "badge.h"

#include <stdio.h>
#include <string.h>

static const char SUFFIX[] = " (visitor)";

size_t badge_line(char *out, size_t cap, const char *name)
{
    size_t nlen = strlen(name);
    size_t slen = sizeof SUFFIX - 1;

    if (nlen + slen < cap) {
        memcpy(out, name, nlen);
        memcpy(out + nlen, SUFFIX, slen + 1);
    } else if (cap > 0) {
        out[0] = '\0';
    }
    return nlen + slen;
}

void badge_print(const char *name)
{
    char line[24];

    badge_line(line, sizeof line, name);
    puts(line);
}
