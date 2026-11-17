/* prefix.c -- s00-e03-literal-write, fixed: the prefix is built in the caller's
 * buffer, which is writable, and only then upper-cased. The level names stay as the
 * module declares them: lower case, read-only, good for the whole run. */
#include "prefix.h"

#include <string.h>

#include "upcase.h"

static const char *level_name(int level)
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
    const char *name = level_name(level);
    size_t len = strlen(name);

    if (len + 3 > cap) /* the name, ':', ' ', and the terminator */
        return -1;
    for (size_t i = 0; i < len; i++)
        out[i] = name[i];
    out[len] = ':';
    out[len + 1] = ' ';
    out[len + 2] = '\0';
    upcase(out);
    return 0;
}
