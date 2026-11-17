/* s04-e01-my-strcat-strncat: harness. Do not edit; write your code in mycat.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mycat.h"
#include "oracle.h"
#include "strcheck.h"

/* dst is a buffer of exactly the bytes the append may write: one byte too many in
 * either direction is a report. src ends at its terminator and nothing follows it. */
static void cat_case(const char *before, const char *src, const char *want, const char *msg)
{
    size_t need = strlen(want) + 1;
    char *dst = guard_new(need);
    char *s = guard_copy(src, strlen(src) + 1);

    memcpy(dst, before, strlen(before) + 1);
    check(my_strcat(dst, s) == dst, "my_strcat returns dst");
    check_bytes(dst, want, need, msg);
    check_guard(dst, "my_strcat writes exactly strlen(src) + 1 bytes");
    guard_free(s);
    guard_free(dst);
}

static void ncat_case(const char *before, const char *src, size_t n, const char *want,
                      const char *msg)
{
    size_t need = strlen(want) + 1;
    char *dst = guard_new(need);
    char *s = guard_copy(src, strlen(src) + 1);

    memcpy(dst, before, strlen(before) + 1);
    check(my_strncat(dst, s, n) == dst, "my_strncat returns dst");
    check_bytes(dst, want, need, msg);
    check_guard(dst, "my_strncat writes at most n + 1 bytes");
    guard_free(s);
    guard_free(dst);
}

/* src with no terminator at all: my_strncat must stop at n and never read src[n]. */
static void ncat_field(const char *bytes, size_t width, const char *before,
                       const char *want, const char *msg)
{
    size_t need = strlen(want) + 1;
    char *dst = guard_new(need);
    char *s = guard_copy(bytes, width);

    memcpy(dst, before, strlen(before) + 1);
    my_strncat(dst, s, width);
    check_bytes(dst, want, need, msg);
    check_guard(dst, "my_strncat writes at most n + 1 bytes");
    guard_free(s);
    guard_free(dst);
}

static void oracle(void)
{
    enum { MAXLEN = 20, CAP = 2 * MAXLEN + 2, CASES = 3000 };
    char a[MAXLEN + 1], b[MAXLEN + 1], ref[CAP];
    char *dst = guard_new(CAP);
    int cat_ok = 1, ncat_ok = 1;

    orc_seed(401);
    for (int i = 0; i < CASES && (cat_ok || ncat_ok); i++) {
        orc_string(a, MAXLEN, ORC_ALPHA_HIGH);
        size_t lb = orc_string(b, MAXLEN, ORC_ALPHA_HIGH);
        size_t n = (size_t)orc_below((unsigned)lb + 2);

        memset(ref, GUARD_FILL, CAP);
        memcpy(ref, a, strlen(a) + 1);
        guard_reset(dst);
        memcpy(dst, a, strlen(a) + 1);
        strcat(ref, b);
        if (my_strcat(dst, b) != dst || memcmp(dst, ref, CAP) != 0) {
            cat_ok = 0;
            printf("       my_strcat differs: seed 401, case %u\n", (unsigned)orc_case());
            printf("       dst was %s, src %s\n", bytes_show(a, strlen(a) + 1), bytes_show(b, lb + 1));
            printf("       yours: %s\n       libc:  %s\n", bytes_show(dst, CAP), bytes_show(ref, CAP));
        }

        memset(ref, GUARD_FILL, CAP);
        memcpy(ref, a, strlen(a) + 1);
        guard_reset(dst);
        memcpy(dst, a, strlen(a) + 1);
        strncat(ref, b, n);
        if (my_strncat(dst, b, n) != dst || memcmp(dst, ref, CAP) != 0) {
            ncat_ok = 0;
            printf("       my_strncat differs: seed 401, case %u, n = %zu\n",
                   (unsigned)orc_case(), n);
            printf("       dst was %s, src %s\n", bytes_show(a, strlen(a) + 1), bytes_show(b, lb + 1));
            printf("       yours: %s\n       libc:  %s\n", bytes_show(dst, CAP), bytes_show(ref, CAP));
        }
    }
    check(cat_ok, "my_strcat agrees with strcat on 3000 generated pairs");
    check(ncat_ok, "my_strncat agrees with strncat on 3000 generated pairs");
    check_guard(dst, "nothing written outside the destination during the generated cases");
    guard_free(dst);
}

int main(void)
{
    cat_case("abc", "def", "abcdef", "my_strcat appends at the terminator");
    cat_case("", "def", "def", "my_strcat onto an empty string");
    cat_case("abc", "", "abc", "my_strcat of an empty string writes only a terminator");
    cat_case("", "", "", "my_strcat of empty onto empty");
    cat_case("caf", "\xC3\xA9", "caf\xC3\xA9", "my_strcat appends bytes above 0x7F");

    ncat_case("abc", "defgh", 3, "abcdef", "my_strncat appends at most n characters");
    ncat_case("abc", "de", 5, "abcde", "my_strncat stops at the source's terminator");
    ncat_case("abc", "defgh", 0, "abc", "my_strncat with n == 0 writes only a terminator");
    ncat_case("", "defgh", 2, "de", "my_strncat onto an empty string");
    ncat_case("abc", "", 4, "abc", "my_strncat of an empty source");
    ncat_case("abc", "defgh", 5, "abcdefgh", "my_strncat with n equal to the source's length");

    /* A source that is a character sequence, not a string. */
    ncat_field("XYZW", 4, "ab", "abXYZW", "my_strncat with n equal to a full field's width");
    ncat_field("XYZW", 4, "", "XYZW", "my_strncat of a full field onto an empty string");

    /* The byte count the README's Behaviour table is about: n + 1 bytes, always. */
    {
        char *dst = guard_new(5);

        memcpy(dst, "ab", 3);
        my_strncat(dst, "cdef", 2);
        check_bytes(dst, "abcd\0", 5, "my_strncat writes n characters and a terminator: n + 1 bytes");
        check_guard(dst, "and not one byte more");
        guard_free(dst);
    }

    oracle();
    return checks_done("s04-e01-my-strcat-strncat");
}
