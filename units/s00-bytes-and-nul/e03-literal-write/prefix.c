/* prefix.c -- s00-e03-literal-write. There is one defect in this file. */
#include "prefix.h"

#include <string.h>

#include "upcase.h"

static char *level_name(int level)
{
    switch (level) {
    case PREFIX_INFO:
        return "info";
    case PREFIX_WARN:
        return "warning";
    default:
        return "error";
    }
}

const char *prefix_name(int level)
{
    return level_name(level);
}

int prefix_write(char *out, size_t cap, int level)
{
    char *name = level_name(level);
    size_t len = strlen(name);

    upcase(name);
    if (len + 3 > cap) /* the name, ':', ' ', and the terminator */
        return -1;
    for (size_t i = 0; i < len; i++)
        out[i] = name[i];
    out[len] = ':';
    out[len + 1] = ' ';
    out[len + 2] = '\0';
    return 0;
}
