/* s04-e02-my-strlcat: harness. Do not edit; write your code in mystrlcat.c. */

/* Every destination is a buffer of exactly `size` bytes, so a write at dst[size] is a
 * fence failure in the plain build and a heap-buffer-overflow under ASan. Every source
 * ends at its terminator with nothing after it. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mystrlcat.h"
#include "oracle.h"
#include "strcheck.h"

/* `before` is the exact contents of the size-byte destination, in bytes, so a test can
 * hand the function a destination with no terminator in it at all. */
static void one(const char *before, size_t size, const char *src, size_t want_ret,
                const char *want, const char *msg)
{
    char *dst = guard_copy(before, size);
    char *s = guard_copy(src, strlen(src) + 1);

    check(my_strlcat(dst, s, size) == want_ret, msg);
    check_bytes(dst, want, size, msg);
    check_guard(dst, "my_strlcat writes nothing outside the destination");
    guard_free(s);
    guard_free(dst);
}

/* The oracle: snprintf("%s%s") produces the same bytes and the same number, for a
 * destination that IS terminated. */
static void oracle(void)
{
    enum { MAXLEN = 16, CAP = 24, CASES = 3000 };
    char a[MAXLEN + 1], b[MAXLEN + 1], ref[CAP + 1];
    int ok = 1;

    orc_seed(402);
    for (int i = 0; i < CASES && ok; i++) {
        size_t la = orc_string(a, MAXLEN, ORC_ALPHA_HIGH);
        size_t lb = orc_string(b, MAXLEN, ORC_ALPHA_HIGH);
        size_t size = (size_t)orc_below(CAP + 1);
        char *dst = guard_new(size);
        size_t mine, theirs;

        if (size == 0) {
            mine = my_strlcat(dst, b, 0);
            if (mine != lb) {
                ok = 0;
                printf("       size 0: yours %zu, want %zu\n", mine, lb);
            }
            guard_free(dst);
            continue;
        }
        /* dst starts as the first min(la, size - 1) characters of a, terminated. */
        size_t keep = la < size - 1 ? la : size - 1;
        memcpy(dst, a, keep);
        dst[keep] = '\0';
        memset(ref, GUARD_FILL, sizeof ref);
        theirs = (size_t)snprintf(ref, size, "%.*s%s", (int)keep, a, b);
        mine = my_strlcat(dst, b, size);
        if (mine != theirs || memcmp(dst, ref, size) != 0) {
            ok = 0;
            printf("       my_strlcat differs from snprintf: seed 402, case %u, size %zu\n",
                   (unsigned)orc_case(), size);
            printf("       dst was %s, src %s\n", bytes_show(a, keep + 1), bytes_show(b, lb + 1));
            printf("       yours: %s (returned %zu)\n", bytes_show(dst, size), mine);
            printf("       libc:  %s (returned %zu)\n", bytes_show(ref, size), theirs);
        }
        check_guard(dst, "my_strlcat writes nothing outside the destination");
        guard_free(dst);
    }
    check(ok, "my_strlcat agrees with snprintf(\"%s%s\") on 3000 generated cases");
}

int main(void)
{
    /* The ordinary cases: the result fits. */
    one("ab\0#####", 8, "cd", 4, "abcd\0###", "the result fits");
    one("\0#######", 8, "cd", 2, "cd\0#####", "appending to an empty string");
    one("ab\0#####", 8, "", 2, "ab\0#####", "appending nothing changes nothing");
    one("abcdefg\0", 8, "", 7, "abcdefg\0", "appending nothing to a full destination");

    /* Truncation: the return value is what it tried to create. */
    one("ab\0#####", 8, "cdefghij", 10, "abcdefg\0", "truncated: returns 2 + 8");
    one("abcdef\0#", 8, "xy", 8, "abcdefx\0", "one character fits");
    one("abcdefg\0", 8, "xy", 9, "abcdefg\0", "nothing fits, and the string is unchanged");
    one("ab\0#####", 8, "cdefg", 7, "abcdefg\0", "the result exactly fills the buffer");

    /* The edges of `size`. */
    one("#", 1, "ab", 3, "#", "size 1 with no terminator: nothing written, reported as size + strlen(src)");
    one("\0", 1, "ab", 2, "\0", "size 1 holding the empty string: nothing fits");
    one("", 0, "ab", 2, "", "size 0: nothing is read or written, and the length of src comes back");

    /* A destination with no terminator among its `size` bytes: the BSD contract says
     * append nothing and report size + strlen(src). */
    one("abcdefgh", 8, "xy", 10, "abcdefgh", "an unterminated destination is left alone");
    one("abcd", 4, "xyz", 7, "abcd", "an unterminated destination, reported as size + strlen(src)");

    /* Bytes above 0x7F are ordinary characters. */
    one("caf\0####", 8, "\xC3\xA9", 5, "caf\xC3\xA9\0##", "appending bytes above 0x7F");

    /* Truncation is detected by comparing the return value with size. */
    {
        char *dst = guard_copy("ab\0#####", 8);
        char *s = guard_copy("cdefghij", 9);
        size_t r = my_strlcat(dst, s, 8);

        check(r >= 8, "a return value >= size means truncated");
        check(r + 1 == 11, "and r + 1 is the size that would have been enough");
        guard_free(s);
        guard_free(dst);
    }

    oracle();
    return checks_done("s04-e02-my-strlcat");
}
