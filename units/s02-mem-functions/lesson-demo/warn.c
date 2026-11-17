/* warn.c -- lesson-demo for S02: the three mistakes of this unit, written where the
 * compiler can see them. Never linked or run; `make capture TOOL=warn` records the
 * diagnostics. */
#include <stdio.h>
#include <string.h>

typedef struct {
    char kind;
    int value;
} Point;

/* 1. memset's value and count, swapped. */
void clear_point(Point *p)
{
    memset(p, sizeof *p, 0);
}

/* 2. memcpy where the two areas are the same object. */
void shift_down(char *buf, size_t len)
{
    memcpy(buf, buf + 1, len - 1);
}

/* 3. sizeof on a pointer, as a byte count. */
void clear_through(char *p)
{
    memset(p, 0, sizeof p);
}
