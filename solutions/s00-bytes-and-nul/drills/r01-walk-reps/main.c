/* s00-r01-walk-reps: harness. Do not edit; write your code in walk.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "oracle.h"
#include "strcheck.h"
#include "walk.h"

/* Every string is a buffer of exactly its own size, so a walk that runs one byte past
 * the terminator is a sanitizer report instead of a lucky answer. */
static void one(const char *s, char c, size_t want_count, int want_last)
{
    size_t len = strlen(s);
    char *only = guard_copy(s, len + 1);

    check(walk_len(only) == len, "walk_len");
    check(walk_end(only) == only + len, "walk_end points at the terminator");
    check(*walk_end(only) == '\0', "the byte walk_end points at is a NUL");
    check(walk_count(only, c) == want_count, "walk_count");
    check(walk_last(only) == want_last, "walk_last");
    check(walk_is_empty(only) == (len == 0), "walk_is_empty");
    guard_free(only);
}

static void oracle(void)
{
    enum { MAXLEN = 32, CASES = 2000 };
    char src[MAXLEN + 1];
    int ok = 1;

    orc_seed(11);
    for (int i = 0; i < CASES && ok; i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, len + 1);
        size_t n_a = 0;

        for (size_t j = 0; j < len; j++) {
            if (only[j] == 'a')
                n_a++;
        }
        if (walk_len(only) != len || walk_end(only) != only + len ||
            walk_count(only, 'a') != n_a ||
            walk_last(only) != (len == 0 ? -1 : (int)(unsigned char)only[len - 1]) ||
            walk_is_empty(only) != (len == 0)) {
            ok = 0;
            printf("       seed 11, case %u, s: %s\n", (unsigned)orc_case(), bytes_show(only, len + 1));
            printf("       walk_len %zu (want %zu), walk_end s + %td (want s + %zu)\n",
                   walk_len(only), len, walk_end(only) - only, len);
            printf("       walk_count(s, 'a') %zu (want %zu), walk_last %d, walk_is_empty %d\n",
                   walk_count(only, 'a'), n_a, walk_last(only), walk_is_empty(only));
        }
        guard_free(only);
    }
    check(ok, "all five agree with a byte-by-byte count on 2000 generated strings");
}

int main(void)
{
    one("", 'x', 0, -1);
    one("x", 'x', 1, 'x');
    one("hello", 'l', 2, 'o');
    one("banana", 'a', 3, 'a');
    one("aaaa", 'a', 4, 'a');
    one("hello", 'z', 0, 'o');
    one("tab\there", '\t', 1, 'e');
    one("caf\xC3\xA9", '\xA9', 1, 0xA9); /* a byte above 0x7F is an ordinary byte */
    one("\xFF\xFF", '\xFF', 2, 0xFF);

    /* The terminator is not a character of the string. */
    {
        char *only = guard_copy("ab", 3);
        check(walk_count(only, '\0') == 0, "walk_count does not count the terminator");
        guard_free(only);
    }

    /* Bytes after the terminator are not part of the string either. */
    {
        char *only = guard_copy("ab\0cd", 6);
        check(walk_len(only) == 2, "walk_len stops at the first terminator");
        check(walk_count(only, 'c') == 0, "walk_count stops at the first terminator");
        check(walk_last(only) == 'b', "walk_last is the byte before the first terminator");
        guard_free(only);
    }

    oracle();
    return checks_done("s00-r01-walk-reps");
}
