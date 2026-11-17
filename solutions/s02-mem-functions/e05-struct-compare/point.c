/* point.c -- s02-e05-struct-compare, fixed: equality is about the members, so it
 * compares the members. The padding between them is not part of the value and its
 * bytes are unspecified, so no byte-wise comparison of the whole object can answer
 * this question.
 *
 * point_copy still uses memcpy, and that is correct: copying the padding along with
 * the members is harmless, and it is what struct assignment does too. */
#include "point.h"

#include <string.h>

Point point_make(char kind, int value)
{
    Point p;

    p.kind = kind;
    p.value = value;
    return p;
}

int point_eq(const Point *a, const Point *b)
{
    return a->kind == b->kind && a->value == b->value;
}

void point_copy(Point *dst, const Point *src)
{
    memcpy(dst, src, sizeof *dst);
}
