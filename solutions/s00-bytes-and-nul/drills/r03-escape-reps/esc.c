/* esc.c -- s00-r03-escape-reps: your four functions. */
#include "esc.h"

/* ---- Your definitions of the esc.h functions go below this line. ---- */

int esc_hex_digit(unsigned v)
{
    if (v < 10)
        return '0' + (int)v;
    if (v < 16)
        return 'A' + (int)(v - 10);
    return -1;
}

/* Appends one character to out, counting it whether or not it fits. */
static void put(char *out, size_t cap, size_t *at, char c)
{
    if (*at + 1 < cap)
        out[*at] = c;
    (*at)++;
}

/* One byte's rendering, appended at *at. */
static void render(char *out, size_t cap, size_t *at, unsigned char b)
{
    switch (b) {
    case '\0':
        put(out, cap, at, '\\');
        put(out, cap, at, '0');
        return;
    case '\n':
        put(out, cap, at, '\\');
        put(out, cap, at, 'n');
        return;
    case '\t':
        put(out, cap, at, '\\');
        put(out, cap, at, 't');
        return;
    case '\r':
        put(out, cap, at, '\\');
        put(out, cap, at, 'r');
        return;
    case '\\':
        put(out, cap, at, '\\');
        put(out, cap, at, '\\');
        return;
    default:
        break;
    }
    if (b >= 0x20 && b <= 0x7E) {
        put(out, cap, at, (char)b);
        return;
    }
    put(out, cap, at, '\\');
    put(out, cap, at, 'x');
    put(out, cap, at, (char)esc_hex_digit(b >> 4));
    put(out, cap, at, (char)esc_hex_digit(b & 0x0Fu));
}

static size_t finish(char *out, size_t cap, size_t at)
{
    if (cap > 0)
        out[at < cap ? at : cap - 1] = '\0';
    return at;
}

size_t esc_one(char *out, size_t cap, unsigned char b)
{
    size_t at = 0;

    render(out, cap, &at, b);
    return finish(out, cap, at);
}

size_t esc_buf(char *out, size_t cap, const void *p, size_t n)
{
    const unsigned char *b = p;
    size_t at = 0;

    for (size_t i = 0; i < n; i++)
        render(out, cap, &at, b[i]);
    return finish(out, cap, at);
}

size_t esc_str(char *out, size_t cap, const char *s)
{
    size_t n = 0;

    while (s[n] != '\0')
        n++;
    return esc_buf(out, cap, s, n + 1);
}
