/* s01-e03-empty-wraps: harness. The defect is in stats.c, not here.
 *
 * Every string is held in a buffer of exactly its own size, so a count that walks past
 * the terminator is a sanitizer report rather than a wrong number.
 */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "stats.h"
#include "strcheck.h"

static void one(const char *s, size_t want_upper, size_t want_digits)
{
    char *only = guard_copy(s, strlen(s) + 1);

    check(count_upper(only) == want_upper, "count_upper");
    check(count_digits(only) == want_digits, "count_digits");
    guard_free(only);
}

int main(void)
{
    one("Hello World", 2, 0);
    one("ABC", 3, 0);
    one("abc", 0, 0);
    one("A1b2C3", 2, 3);
    one("x", 0, 0);
    one("2026-11-16", 0, 8);
    one("caf\xC3\xA9", 0, 0); /* bytes above 0x7F are not letters or digits */
    one("", 0, 0);            /* the empty string: zero of everything */

    return checks_done("s01-e03-empty-wraps");
}
