/* s03-e02-my-strncpy: harness. Do not edit; write your code in mystrncpy.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mystrncpy.h"
#include "oracle.h"
#include "strcheck.h"

static int cases_ok = 1;

/* One (src, n) pair against libc. The destination has exactly n bytes; the source
 * buffer holds exactly the bytes strncpy is allowed to read: the string and its
 * terminator if it is shorter than n, else its first n characters and no terminator. */
static void one_case(const char *src, size_t n, const char *where)
{
    size_t len = strlen(src);
    size_t readable = len < n ? len + 1 : n;
    char *only = guard_copy(src, readable);
    char *dst = guard_new(n);
    char ref[64];

    memset(ref, GUARD_FILL, sizeof ref);
    strncpy(ref, src, n);
    char *r = my_strncpy(dst, only, n);
    if (r != dst || memcmp(dst, ref, n) != 0 || !guard_ok(dst)) {
        if (cases_ok) {
            printf("       my_strncpy(dst, src, %zu) differs from strncpy (%s); src: %s\n", n, where,
                   bytes_show(src, len + 1));
            printf("       yours: %s%s%s\n", bytes_show(dst, n), r == dst ? "" : "  (did not return dst)",
                   guard_ok(dst) ? "" : "  (wrote outside the n bytes)");
            printf("       libc:  %s\n", bytes_show(ref, n));
        }
        cases_ok = 0;
    }

    memset(ref, GUARD_FILL, sizeof ref);
    char *e = stpncpy(ref, src, n);
    guard_reset(dst);
    r = my_stpncpy(dst, only, n);
    if (r - dst != e - ref || memcmp(dst, ref, n) != 0 || !guard_ok(dst)) {
        if (cases_ok) {
            printf("       my_stpncpy(dst, src, %zu) differs from stpncpy (%s); src: %s\n", n, where,
                   bytes_show(src, len + 1));
            printf("       yours: %s  returned dst + %td%s\n", bytes_show(dst, n), r - dst,
                   guard_ok(dst) ? "" : "  (wrote outside the n bytes)");
            printf("       libc:  %s  returned dst + %td\n", bytes_show(ref, n), e - ref);
        }
        cases_ok = 0;
    }
    guard_free(dst);
    guard_free(only);
}

int main(void)
{
    /* The cases of the README's Behaviour table, checked one by one. */
    char *dst = guard_new(8);

    check(my_strncpy(dst, "hello", 8) == dst, "my_strncpy returns dst");
    check_bytes(dst, "hello\0\0\0", 8, "short source: copied, then padded with NULs to n bytes");
    guard_reset(dst);
    my_strncpy(dst, "hello", 5);
    check_bytes(dst, "hello###", 8, "source of exactly n characters: n bytes, no terminator");
    guard_reset(dst);
    my_strncpy(dst, "hello", 3);
    check_bytes(dst, "hel#####", 8, "source longer than n: first n characters, no terminator");
    guard_reset(dst);
    my_strncpy(dst, "", 4);
    check_bytes(dst, "\0\0\0\0####", 8, "empty source: n NUL bytes");
    guard_reset(dst);
    my_strncpy(dst, "hello", 0);
    check_bytes(dst, "########", 8, "n == 0 writes nothing");
    guard_reset(dst);
    my_strncpy(dst, "abcdefgh", 8);
    check_bytes(dst, "abcdefgh", 8, "a full field has no terminator");
    guard_reset(dst);
    my_strncpy(dst, "a\xFFz", 8);
    check_bytes(dst, "a\xFFz\0\0\0\0\0", 8, "a byte above 0x7F is an ordinary character, not the end");

    guard_reset(dst);
    check(my_stpncpy(dst, "hey", 8) == dst + 3, "my_stpncpy returns the first NUL it wrote");
    check_bytes(dst, "hey\0\0\0\0\0", 8, "my_stpncpy pads like my_strncpy");
    guard_reset(dst);
    check(my_stpncpy(dst, "hello", 5) == dst + 5, "my_stpncpy returns dst + n when no NUL was written");
    check_bytes(dst, "hello###", 8, "my_stpncpy with a source of exactly n characters");
    guard_reset(dst);
    check(my_stpncpy(dst, "", 4) == dst, "my_stpncpy with an empty source returns dst");
    guard_reset(dst);
    check(my_stpncpy(dst, "abc", 0) == dst, "my_stpncpy with n == 0 returns dst");
    check_bytes(dst, "########", 8, "my_stpncpy with n == 0 writes nothing");
    check_guard(dst, "nothing written outside the 8-byte buffer");
    guard_free(dst);

    /* Every source length 0..10 against every n 0..12, each in an exact-size buffer. */
    static const char letters[] = "abcdefghij";
    char src[16];
    for (size_t len = 0; len <= 10; len++) {
        memcpy(src, letters, len);
        src[len] = '\0';
        for (size_t n = 0; n <= 12; n++)
            one_case(src, n, "grid of every length 0..10 and every n 0..12");
    }
    check(cases_ok, "agrees with libc for every source length 0..10 and every n 0..12");

    /* Generated strings, with bytes above 0x7F. */
    orc_seed(302);
    for (int i = 0; i < 2000 && cases_ok; i++) {
        orc_string(src, 15, ORC_ALPHA_HIGH);
        one_case(src, orc_below(20), "generated, seed 302");
    }
    check(cases_ok, "agrees with libc on 2000 generated (string, n) pairs");

    return checks_done("s03-e02-my-strncpy");
}
