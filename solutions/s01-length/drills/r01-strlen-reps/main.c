/* s01-r01-strlen-reps: harness. Do not edit; write your code in lenreps.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "lenreps.h"
#include "oracle.h"
#include "strcheck.h"

static void one(const char *s)
{
    size_t want = strlen(s);
    char *only = guard_copy(s, want + 1);

    check(len_index(only) == want, "len_index agrees with strlen");
    check(len_diff(only) == want, "len_diff agrees with strlen");
    check(len_bounded(only, want + 1) == want, "len_bounded with room to find the terminator");
    check(len_bounded(only, 0) == 0, "len_bounded with maxlen 0");
    check(len_fits(only, want + 1) == 1, "a string fits in exactly its own size");
    check(len_fits(only, want) == 0, "and not in one byte less");
    check_guard(only, "nothing was written");
    guard_free(only);
}

/* A character sequence with no terminator: only the bounded function may be used. */
static void field(const char *bytes, size_t width)
{
    char *only = guard_copy(bytes, width);

    for (size_t m = 0; m <= width; m++)
        check(len_bounded(only, m) == (m < width ? m : width),
              "len_bounded of a full field is maxlen, up to the field's width");
    guard_free(only);
}

static void oracle(void)
{
    enum { MAXLEN = 40, CASES = 2000 };
    char src[MAXLEN + 1];
    int ok = 1;

    orc_seed(7);
    for (int i = 0; i < CASES && ok; i++) {
        size_t want = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, want + 1);
        size_t maxlen = (size_t)orc_below((unsigned)want + 3);

        if (len_index(only) != want || len_diff(only) != want ||
            len_bounded(only, maxlen) != strnlen(only, maxlen) ||
            len_fits(only, want + 1) != 1 || len_fits(only, want) != 0) {
            ok = 0;
            printf("       seed 7, case %u, maxlen %zu, s: %s\n",
                   (unsigned)orc_case(), maxlen, bytes_show(only, want + 1));
            printf("       len_index %zu, len_diff %zu, len_bounded %zu (libc %zu), len_fits %d/%d\n",
                   len_index(only), len_diff(only), len_bounded(only, maxlen),
                   strnlen(only, maxlen), len_fits(only, want + 1), len_fits(only, want));
        }
        guard_free(only);
    }
    check(ok, "all four agree with libc on 2000 generated strings");
}

int main(void)
{
    one("");
    one("x");
    one("hello");
    one("a longer string, with spaces and 1234567890");
    one("caf\xC3\xA9 \xFF\x80");

    field("A123", 4);
    field("\xFF\xE9\x80", 3);
    field("z", 1);

    {   /* The empty string needs one byte, and cap == 0 holds nothing at all. */
        char *empty = guard_copy("", 1);
        check(len_fits(empty, 1) == 1, "the empty string fits in one byte");
        check(len_fits(empty, 0) == 0, "nothing fits in zero bytes");
        guard_free(empty);
    }

    {   /* An embedded NUL ends the string for all four. */
        char *only = guard_copy("ab\0cd", 6);
        check(len_index(only) == 2, "len_index stops at an embedded NUL");
        check(len_diff(only) == 2, "len_diff stops at an embedded NUL");
        check(len_bounded(only, 6) == 2, "len_bounded stops at an embedded NUL");
        check(len_fits(only, 3) == 1, "\"ab\" fits in three bytes");
        guard_free(only);
    }

    oracle();
    return checks_done("s01-r01-strlen-reps");
}
