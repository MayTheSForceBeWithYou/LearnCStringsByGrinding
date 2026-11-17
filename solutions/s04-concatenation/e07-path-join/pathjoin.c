/* pathjoin.c -- s04-e07-path-join: your two functions. */
#include "pathjoin.h"

#include <stdlib.h>
#include <string.h>

/* ---- Your definitions of the pathjoin.h functions go below this line. ---- */

/* The part's length with any trailing '/' removed. */
static size_t trimmed_end(const char *s)
{
    size_t n = strlen(s);

    while (n > 0 && s[n - 1] == '/')
        n--;
    return n;
}

/* The offset of the part's first byte that is not '/'. */
static size_t trimmed_begin(const char *s)
{
    size_t i = 0;

    while (s[i] == '/')
        i++;
    return i;
}

/* One pass over the parts, with the separator rule in one place. When `out` is NULL
 * nothing is written and the return value is the length the result needs; when it is
 * not, the result is written there and the return value is the same number. The two
 * passes cannot disagree, because they are the same code. */
static size_t build(char *out, const char *const *parts, size_t n)
{
    size_t at = 0;
    int ends_slash = 0, any = 0;

    for (size_t i = 0; i < n; i++) {
        size_t b = trimmed_begin(parts[i]);
        size_t e = trimmed_end(parts[i]);
        size_t lead = (i == 0) ? b : 0;
        size_t body = e > b ? e - b : 0;

        if (lead > 0) {
            if (out != NULL)
                memset(out + at, '/', lead);
            at += lead;
            ends_slash = 1;
            any = 1;
        }
        if (body > 0) {
            if (any && !ends_slash) {
                if (out != NULL)
                    out[at] = '/';
                at++;
            }
            if (out != NULL)
                memcpy(out + at, parts[i] + b, body);
            at += body;
            ends_slash = 0;
            any = 1;
        }
    }
    if (out != NULL)
        out[at] = '\0';
    return at;
}

char *path_join_n(const char *const *parts, size_t n)
{
    size_t total = build(NULL, parts, n);
    char *out = malloc(total + 1);

    if (out == NULL)
        return NULL;
    build(out, parts, n);
    return out;
}

char *path_join(const char *dir, const char *name)
{
    const char *const parts[2] = {dir, name};

    return path_join_n(parts, 2);
}
