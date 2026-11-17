/* app.c -- s04-r01-append-reps: your four functions. */
#include "app.h"

#include <string.h>

/* ---- Your definitions of the app.h functions go below this line. ---- */

size_t app_room(const char *p, const char *end)
{
    return p < end ? (size_t)(end - p) - 1 : 0;
}

char *app_str(char *p, char *end, const char *s)
{
    size_t room = app_room(p, end);
    size_t len = strlen(s);
    size_t keep = len < room ? len : room;

    memcpy(p, s, keep);
    p += keep;
    *p = '\0';
    return p;
}

char *app_char(char *p, char *end, char c)
{
    if (app_room(p, end) == 0)
        return p;
    *p++ = c;
    *p = '\0';
    return p;
}

char *app_dec(char *p, char *end, long v)
{
    char digits[24];
    size_t n = 0;
    unsigned long u;

    if (v < 0) {
        p = app_char(p, end, '-');
        u = 0UL - (unsigned long)v; /* correct for LONG_MIN, which cannot be negated */
    } else {
        u = (unsigned long)v;
    }
    do {
        digits[n++] = (char)('0' + (int)(u % 10));
        u /= 10;
    } while (u > 0);
    while (n > 0) {
        char *q = app_char(p, end, digits[--n]);

        if (q == p)
            break; /* no room left: stop, leaving what fitted */
        p = q;
    }
    return p;
}
