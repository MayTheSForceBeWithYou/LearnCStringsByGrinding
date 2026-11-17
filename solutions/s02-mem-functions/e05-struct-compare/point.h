/* point.h -- s02-e05-struct-compare: a small record with a hole in it.
 *
 * The compiler is allowed to insert padding between members so that each one is
 * suitably aligned (C17 6.7.2.1p15). On both course platforms this struct is 8 bytes:
 * one byte of `kind`, three bytes of padding, and four bytes of `value`. The padding
 * bytes are part of the object and their values are unspecified.
 */
#ifndef POINT_H
#define POINT_H

typedef struct {
    char kind;
    int value;
} Point;

/* A point with the given kind and value. */
Point point_make(char kind, int value);

/* 1 if the two points have the same kind and the same value, else 0. */
int point_eq(const Point *a, const Point *b);

/* Copies src into dst. */
void point_copy(Point *dst, const Point *src);

#endif
