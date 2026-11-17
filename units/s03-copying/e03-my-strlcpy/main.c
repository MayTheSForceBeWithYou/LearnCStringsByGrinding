/* s03-e03-my-strlcpy: harness. Do not edit; write your code in mystrlcpy.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mystrlcpy.h"
#include "oracle.h"
#include "strcheck.h"

static int cases_ok = 1;

/* The BSD function this reimplements cannot be called here, so libc's snprintf is the oracle:
 * snprintf(dst, size, "%s", src) writes the same bytes and returns the same number. */
static void one_case(const char *src, size_t size, const char *where)
{
    size_t len = strlen(src);
    char *only = guard_copy(src, len + 1);
    char *dst = guard_new(size);
    char ref[64];

    memset(ref, GUARD_FILL, sizeof ref);
    int want = snprintf(ref, size, "%s", src);
    size_t got = my_strlcpy(dst, only, size);
    if (got != (size_t)want || memcmp(dst, ref, size) != 0 || !guard_ok(dst)) {
        if (cases_ok) {
            printf("       my_strlcpy(dst, src, %zu) is wrong (%s); src: %s\n", size, where,
                   bytes_show(src, len + 1));
            printf("       yours: %s  returned %zu%s\n", bytes_show(dst, size), got,
                   guard_ok(dst) ? "" : "  (wrote outside the buffer)");
            printf("       want:  %s  returned %d\n", bytes_show(ref, size), want);
        }
        cases_ok = 0;
    }
    guard_free(dst);
    guard_free(only);
}

int main(void)
{
    /* The cases of the README's Behaviour table. */
    char *dst = guard_new(8);

    check(my_strlcpy(dst, "hello", 8) == 5, "fits: returns strlen(src)");
    check_bytes(dst, "hello\0##", 8, "fits: copied and terminated, no padding");
    guard_reset(dst);
    check(my_strlcpy(dst, "abcdefg", 8) == 7, "exact fit (7 characters in 8 bytes): returns 7");
    check_bytes(dst, "abcdefg\0", 8, "exact fit: the terminator is the last byte");
    guard_reset(dst);
    check(my_strlcpy(dst, "abcdefgh", 8) == 8, "one too many: returns 8, which is >= size");
    check_bytes(dst, "abcdefg\0", 8, "one too many: 7 characters and a terminator");
    guard_reset(dst);
    check(my_strlcpy(dst, "abcdefghijkl", 8) == 12, "truncated: still returns the full source length");
    check_bytes(dst, "abcdefg\0", 8, "truncated: size - 1 characters and a terminator");
    guard_reset(dst);
    check(my_strlcpy(dst, "", 8) == 0, "empty source: returns 0");
    check_bytes(dst, "\0#######", 8, "empty source: one terminator, nothing else");
    guard_reset(dst);
    check(my_strlcpy(dst, "hello", 1) == 5, "size 1: returns the source length");
    check_bytes(dst, "\0#######", 8, "size 1: room for the terminator only");
    guard_reset(dst);
    check(my_strlcpy(dst, "hello", 0) == 5, "size 0: still returns the source length");
    check_bytes(dst, "########", 8, "size 0: nothing is written");
    guard_reset(dst);
    check(my_strlcpy(dst, "a\xFFz", 3) == 3, "bytes above 0x7F are ordinary characters");
    check_bytes(dst, "a\xFF\0#####", 8, "truncation after a byte above 0x7F");
    check_guard(dst, "nothing written outside the 8-byte buffer");
    guard_free(dst);

    /* The idiom the return value exists for. */
    char small[4];
    check(my_strlcpy(small, "abc", sizeof small) < sizeof small, "\"abc\" in 4 bytes: not truncated");
    check(my_strlcpy(small, "abcd", sizeof small) >= sizeof small, "\"abcd\" in 4 bytes: truncated");
    check_str(small, "abc", "after a truncated copy the buffer still holds a string");

    /* Every source length 0..10 against every size 0..12, in exact-size buffers. */
    static const char letters[] = "abcdefghij";
    char src[24];
    for (size_t len = 0; len <= 10; len++) {
        memcpy(src, letters, len);
        src[len] = '\0';
        for (size_t size = 0; size <= 12; size++)
            one_case(src, size, "grid of every length 0..10 and every size 0..12");
    }
    check(cases_ok, "correct for every source length 0..10 and every size 0..12");

    orc_seed(303);
    for (int i = 0; i < 2000 && cases_ok; i++) {
        orc_string(src, 20, ORC_ALPHA_HIGH);
        one_case(src, orc_below(26), "generated, seed 303");
    }
    check(cases_ok, "correct on 2000 generated (string, size) pairs");

    return checks_done("s03-e03-my-strlcpy");
}
