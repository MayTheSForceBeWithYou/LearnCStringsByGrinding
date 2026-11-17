/* cursor.c -- s04-e05-append-cursor: your six functions. */
#include "cursor.h"

#include <stdio.h>
#include <string.h>

/* ---- Your definitions of the cursor.h functions go below this line. ---- */

void cur_init(Cursor *c, char *buf, size_t cap)
{
    c->buf = buf;
    c->cap = cap;
    c->len = 0;
    c->truncated = 0;
    if (cap > 0)
        buf[0] = '\0';
}

/* The room for characters at the cursor's position, terminator reserved. */
static size_t room(const Cursor *c)
{
    return c->cap > c->len + 1 ? c->cap - c->len - 1 : 0;
}

void cur_puts(Cursor *c, const char *s)
{
    size_t slen = strlen(s);
    size_t r = room(c);
    size_t keep = slen < r ? slen : r;

    if (c->cap == 0) {
        if (slen > 0)
            c->truncated = 1;
        return;
    }
    memcpy(c->buf + c->len, s, keep);
    c->len += keep;
    c->buf[c->len] = '\0';
    if (keep < slen)
        c->truncated = 1;
}

void cur_putc(Cursor *c, char ch)
{
    if (c->cap == 0 || room(c) == 0) {
        c->truncated = 1;
        return;
    }
    c->buf[c->len++] = ch;
    c->buf[c->len] = '\0';
}

void cur_printf(Cursor *c, const char *fmt, ...)
{
    va_list ap;
    int n;

    if (c->cap == 0) {
        c->truncated = 1;
        return;
    }
    va_start(ap, fmt);
    n = vsnprintf(c->buf + c->len, c->cap - c->len, fmt, ap);
    va_end(ap);
    if (n < 0) {
        c->buf[c->len] = '\0';
        c->truncated = 1;
        return;
    }
    if ((size_t)n >= c->cap - c->len) {
        c->len = c->cap - 1;
        c->truncated = 1;
    } else {
        c->len += (size_t)n;
    }
}

int cur_truncated(const Cursor *c)
{
    return c->truncated;
}

size_t cur_len(const Cursor *c)
{
    return c->len;
}
