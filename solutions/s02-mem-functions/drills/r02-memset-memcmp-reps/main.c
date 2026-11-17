/* s02-r02-memset-memcmp-reps: harness. Do not edit; write your code in zero.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "oracle.h"
#include "strcheck.h"
#include "zero.h"

typedef struct {
    int id;
    char tag[6];
} Rec;

static void eq_case(const char *a, const char *b, size_t n, int want, const char *msg)
{
    char *pa = guard_copy(a, n);
    char *pb = guard_copy(b, n);

    check(ranges_equal(pa, pb, n) == want, msg);
    check(ranges_equal(pb, pa, n) == want, "and the other way round");
    guard_free(pb);
    guard_free(pa);
}

static void prefix_case(const char *a, const char *b, size_t n, size_t want, const char *msg)
{
    char *pa = guard_copy(a, n);
    char *pb = guard_copy(b, n);

    check(common_prefix(pa, pb, n) == want, msg);
    check(common_prefix(pb, pa, n) == want, "and the other way round");
    check_guard(pa, "common_prefix writes nothing");
    guard_free(pb);
    guard_free(pa);
}

static void all_case(const char *bytes, size_t n, int c, int want, const char *msg)
{
    char *p = guard_copy(bytes, n);

    check(all_equal_to(p, c, n) == want, msg);
    guard_free(p);
}

static void oracle(void)
{
    enum { MAXLEN = 24, CASES = 2000 };
    char a[MAXLEN + 1], b[MAXLEN + 1];
    int ok = 1;

    orc_seed(212);
    for (int i = 0; i < CASES && ok; i++) {
        size_t la = orc_string(a, MAXLEN, ORC_ALPHA_AB);
        size_t lb = orc_string(b, MAXLEN, ORC_ALPHA_AB);
        size_t n = la < lb ? la : lb;
        char *pa = guard_copy(a, n);
        char *pb = guard_copy(b, n);
        size_t want = 0;

        while (want < n && pa[want] == pb[want])
            want++;
        if (ranges_equal(pa, pb, n) != (memcmp(pa, pb, n) == 0) ||
            common_prefix(pa, pb, n) != want) {
            ok = 0;
            printf("       seed 212, case %u, n = %zu\n", (unsigned)orc_case(), n);
            printf("       a: %s\n       b: %s\n", bytes_show(pa, n), bytes_show(pb, n));
            printf("       ranges_equal %d (want %d), common_prefix %zu (want %zu)\n",
                   ranges_equal(pa, pb, n), memcmp(pa, pb, n) == 0,
                   common_prefix(pa, pb, n), want);
        }
        guard_free(pb);
        guard_free(pa);
    }
    check(ok, "ranges_equal and common_prefix agree with libc on 2000 generated pairs");
}

int main(void)
{
    /* zero_array over an array of structs, sized with sizeof *p. */
    {
        Rec *table = (Rec *)guard_new(3 * sizeof(Rec));

        memset(table, 0x5A, 3 * sizeof(Rec));
        zero_array(table, 3, sizeof *table);
        check(all_equal_to(table, 0, 3 * sizeof(Rec)), "zero_array clears every byte, padding included");
        check_guard((char *)table, "zero_array writes nothing outside the array");
        guard_free((char *)table);
    }
    {
        char *buf = guard_new(8);

        memset(buf, 'x', 8);
        zero_array(buf, 0, 4);
        check(all_equal_to(buf, 'x', 8), "zero_array with n == 0 writes nothing");
        zero_array(buf, 4, 0);
        check(all_equal_to(buf, 'x', 8), "zero_array with size == 0 writes nothing");
        zero_array(buf, 2, 3);
        check_bytes(buf, "\0\0\0\0\0\0xx", 8, "zero_array clears n * size bytes");
        guard_free(buf);
    }

    /* ranges_equal */
    eq_case("abc", "abc", 3, 1, "equal ranges");
    eq_case("abc", "abd", 3, 0, "ranges differing in the last byte");
    eq_case("abc", "abd", 2, 1, "the difference past n does not count");
    eq_case("a", "b", 0, 1, "two empty ranges are equal");
    eq_case("\x00\xFF", "\x00\xFF", 2, 1, "a NUL byte is compared like any other");
    eq_case("\x7F", "\x80", 1, 0, "0x7F and 0x80 differ");

    /* common_prefix */
    prefix_case("abcdef", "abcxyz", 6, 3, "a prefix of three");
    prefix_case("abc", "xbc", 3, 0, "no common prefix");
    prefix_case("abc", "abc", 3, 3, "equal ranges share everything");
    prefix_case("abc", "abd", 2, 2, "n limits the answer");
    prefix_case("a", "b", 0, 0, "an empty range shares nothing");
    prefix_case("a\0c", "a\0d", 3, 2, "a NUL byte does not stop the comparison");

    /* all_equal_to */
    all_case("aaaa", 4, 'a', 1, "all four bytes equal");
    all_case("aaab", 4, 'a', 0, "one byte differs");
    all_case("baaa", 4, 'a', 0, "the first byte differs");
    all_case("", 0, 'a', 1, "an empty range is all anything");
    all_case("\0\0", 2, 0, 1, "all NUL bytes");
    all_case("\xE9\xE9", 2, 0xE9, 1, "all high bytes");
    all_case("\xE9\xE9", 2, 0x141 + 0xA8, 1, "the value is converted to unsigned char: 0x1E9 is 0xE9");

    oracle();
    return checks_done("s02-r02-memset-memcmp-reps");
}
