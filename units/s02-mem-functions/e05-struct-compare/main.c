/* s02-e05-struct-compare: harness. The defect is in point.c, not here.
 *
 * Two points are built from the same kind and value by two different routes: one from
 * a variable whose storage held something else first, one from storage that was zeroed.
 * They are equal by every statement point.h makes about a Point. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "point.h"
#include "strcheck.h"

/* A Point made in storage that has been scribbled on first. The members are set
 * through point_make, so every member has the value the caller asked for; only the
 * padding between them keeps whatever was there. */
static Point dirty_point(char kind, int value, unsigned char fill)
{
    unsigned char raw[sizeof(Point)];
    Point p;

    memset(raw, fill, sizeof raw);
    memcpy(&p, raw, sizeof p);
    p.kind = kind;
    p.value = value;
    return p;
}

int main(void)
{
    check(sizeof(Point) > sizeof(char) + sizeof(int) - 1, "a Point is at least kind + value");

    {   /* The same kind and value, built two ways. */
        Point a = dirty_point('p', 42, 0x00);
        Point b = dirty_point('p', 42, 0x5A);

        check(a.kind == b.kind && a.value == b.value, "the two points have the same members");
        check(point_eq(&a, &b), "two points with the same kind and value are equal");
        check(point_eq(&b, &a), "and equality does not depend on the order");
    }

    {   /* Different kinds, and different values. */
        Point a = point_make('p', 42);
        Point b = point_make('q', 42);
        Point c = point_make('p', 43);

        check(!point_eq(&a, &b), "points with different kinds are not equal");
        check(!point_eq(&a, &c), "points with different values are not equal");
        check(point_eq(&a, &a), "a point equals itself");
    }

    {   /* A copy is equal to its original, whatever the padding held. */
        Point a = dirty_point('z', -7, 0xC3);
        Point b;

        memset(&b, 0x11, sizeof b);
        point_copy(&b, &a);
        check(b.kind == 'z' && b.value == -7, "point_copy copies the members");
        check(point_eq(&a, &b), "a copy is equal to its original");
    }

    {   /* A table of points, searched for one built elsewhere. */
        Point table[3];
        Point key = dirty_point('b', 2, 0x77);
        int found = 0;

        table[0] = dirty_point('a', 1, 0x00);
        table[1] = dirty_point('b', 2, 0xFF);
        table[2] = dirty_point('c', 3, 0x5A);
        for (int i = 0; i < 3; i++) {
            if (point_eq(&table[i], &key))
                found = 1;
        }
        check(found, "a point built elsewhere is found in the table");
    }

    return checks_done("s02-e05-struct-compare");
}
