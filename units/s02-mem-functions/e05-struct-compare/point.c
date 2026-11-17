/* point.c -- s02-e05-struct-compare. There is one defect in this file. */
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
    return memcmp(a, b, sizeof *a) == 0;
}

void point_copy(Point *dst, const Point *src)
{
    memcpy(dst, src, sizeof *dst);
}
