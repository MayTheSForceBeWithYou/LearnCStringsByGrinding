/* s02-e02-my-memset-memcmp-memchr: harness. Do not edit; write your code in mymem.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mymem.h"
#include "oracle.h"
#include "strcheck.h"

static void set_case(size_t cap, int c, size_t n, const char *want, const char *msg)
{
    char *dst = guard_new(cap);

    check(my_memset(dst, c, n) == dst, "my_memset returns s");
    check_bytes(dst, want, cap, msg);
    check_guard(dst, "my_memset writes exactly n bytes");
    guard_free(dst);
}

/* Both sides are buffers of exactly n bytes: a comparison that reads one byte too far
 * is a report, not a lucky match. */
static void cmp_case(const char *a, const char *b, size_t n, int want_sign, const char *msg)
{
    char *pa = guard_copy(a, n);
    char *pb = guard_copy(b, n);

    check(sign_of(my_memcmp(pa, pb, n)) == want_sign, msg);
    check(sign_of(my_memcmp(pb, pa, n)) == -want_sign, "and the reverse comparison has the opposite sign");
    check(sign_of(my_memcmp(pa, pb, n)) == sign_of(memcmp(pa, pb, n)), "and it agrees with memcmp");
    guard_free(pb);
    guard_free(pa);
}

static void chr_case(const char *bytes, size_t n, int c, long want_offset, const char *msg)
{
    char *s = guard_copy(bytes, n);
    void *got = my_memchr(s, c, n);

    if (want_offset < 0)
        check(got == NULL, msg);
    else
        check(got == s + want_offset, msg);
    check_guard(s, "my_memchr writes nothing");
    guard_free(s);
}

static void oracle(void)
{
    enum { MAXLEN = 32, CASES = 3000 };
    char a[MAXLEN + 1], b[MAXLEN + 1];
    int cmp_ok = 1, chr_ok = 1, set_ok = 1;

    orc_seed(202);
    for (int i = 0; i < CASES && (cmp_ok && chr_ok && set_ok); i++) {
        size_t la = orc_string(a, MAXLEN, ORC_ALPHA_AB);
        size_t lb = orc_string(b, MAXLEN, ORC_ALPHA_AB);
        size_t n = la < lb ? la : lb;
        char *pa = guard_copy(a, n);
        char *pb = guard_copy(b, n);
        int c = (int)(unsigned char)"ab\xE9"[orc_below(3)];

        if (sign_of(my_memcmp(pa, pb, n)) != sign_of(memcmp(pa, pb, n))) {
            cmp_ok = 0;
            printf("       my_memcmp differs: seed 202, case %u, n = %zu\n", (unsigned)orc_case(), n);
            printf("       a: %s\n       b: %s\n", bytes_show(pa, n), bytes_show(pb, n));
            printf("       yours: %d   libc: %d\n", sign_of(my_memcmp(pa, pb, n)),
                   sign_of(memcmp(pa, pb, n)));
        }
        {
            void *mine = my_memchr(pa, c, n);
            void *theirs = memchr(pa, c, n);

            if (mine != theirs) {
                chr_ok = 0;
                printf("       my_memchr differs: seed 202, case %u, n = %zu, c = %s\n",
                       (unsigned)orc_case(), n, bytes_show(&c, 1));
                printf("       a: %s\n", bytes_show(pa, n));
                printf("       yours: %s   libc: %s\n", mine ? "found" : "NULL",
                       theirs ? "found" : "NULL");
            }
        }
        {
            char *dst = guard_new(n);
            char ref[MAXLEN + 1];

            memset(ref, c, n);
            my_memset(dst, c, n);
            if (memcmp(dst, ref, n) != 0) {
                set_ok = 0;
                printf("       my_memset differs: seed 202, case %u, n = %zu\n", (unsigned)orc_case(), n);
            }
            guard_free(dst);
        }
        guard_free(pb);
        guard_free(pa);
    }
    check(cmp_ok, "my_memcmp agrees in sign with memcmp on 3000 generated pairs");
    check(chr_ok, "my_memchr agrees with memchr on 3000 generated buffers");
    check(set_ok, "my_memset agrees with memset on 3000 generated sizes");
}

int main(void)
{
    /* my_memset: the value is converted to unsigned char. */
    set_case(8, '#', 8, "########", "my_memset fills the whole buffer");
    set_case(8, 'x', 3, "xxx#####", "my_memset fills only the first n bytes");
    set_case(8, 'x', 0, "########", "my_memset with n == 0 writes nothing");
    set_case(8, 0, 4, "\0\0\0\0####", "my_memset writes NUL bytes for c == 0");
    set_case(8, 0xE9, 2, "\xE9\xE9######", "my_memset stores a byte above 0x7F");
    set_case(8, 0x141, 4, "AAAA####", "my_memset converts c to unsigned char (0x141 is 'A')");
    set_case(8, -1, 2, "\xFF\xFF######", "my_memset converts a negative c to unsigned char");
    set_case(1, 'z', 1, "z", "my_memset of a one-byte buffer");
    {
        char *dst = guard_new(0);
        check(my_memset(dst, 'x', 0) == dst, "my_memset with a zero-byte buffer returns s");
        guard_free(dst);
    }

    /* my_memcmp: only the sign is specified, and bytes compare as unsigned char. */
    cmp_case("abc", "abc", 3, 0, "equal buffers compare equal");
    cmp_case("abc", "abd", 3, -1, "a smaller byte in a is negative");
    cmp_case("abd", "abc", 3, 1, "a larger byte in a is positive");
    cmp_case("abc", "abd", 2, 0, "the difference past n is not compared");
    cmp_case("a", "a", 0, 0, "n == 0 compares equal and reads nothing");
    cmp_case("\x7F", "\x80", 1, -1, "0x7F is smaller than 0x80: bytes compare as unsigned char");
    cmp_case("\xFF", "\x01", 1, 1, "0xFF is larger than 0x01, not smaller");
    cmp_case("a\0b", "a\0c", 3, -1, "a NUL byte does not end the comparison");
    cmp_case("ab", "abc", 2, 0, "the first n bytes are all that matter");

    /* my_memchr: NULL when not found, and a NUL byte is an ordinary byte. */
    chr_case("hello", 5, 'l', 2, "my_memchr finds the first match");
    chr_case("hello", 5, 'h', 0, "my_memchr finds a match at the first byte");
    chr_case("hello", 5, 'o', 4, "my_memchr finds a match at the last byte");
    chr_case("hello", 5, 'z', -1, "my_memchr returns NULL when there is no match");
    chr_case("hello", 3, 'l', 2, "my_memchr respects n");
    chr_case("hello", 2, 'l', -1, "my_memchr does not look past n");
    chr_case("ab\0cd", 5, 0, 2, "my_memchr finds a NUL byte");
    chr_case("ab\0cd", 5, 'd', 4, "my_memchr keeps looking after a NUL byte");
    chr_case("a\xE9z", 3, 0xE9, 1, "my_memchr finds a byte above 0x7F");
    chr_case("a\xE9z", 3, 0x141 + 0x14, -1, "my_memchr converts c to unsigned char");
    chr_case("hello", 0, 'h', -1, "my_memchr with n == 0 is NULL and reads nothing");
    {
        char *s = guard_new(0);
        check(my_memchr(s, 'x', 0) == NULL, "my_memchr on a zero-byte buffer is NULL");
        guard_free(s);
    }

    /* The three together: fill, search, compare. */
    {
        char *a = guard_new(16);
        char *b = guard_new(16);

        my_memset(a, '-', 16);
        my_memset(b, '-', 16);
        check(my_memcmp(a, b, 16) == 0, "two buffers filled the same way compare equal");
        check(my_memchr(a, '-', 16) == a, "and the fill byte is found at the first byte");
        check(my_memchr(a, '+', 16) == NULL, "and no other byte is found");
        my_memset(b + 8, '+', 1);
        check(sign_of(my_memcmp(a, b, 16)) == 1, "'-' (45) is larger than '+' (43)");
        check(my_memchr(b, '+', 16) == b + 8, "and the changed byte is where it was written");
        guard_free(b);
        guard_free(a);
    }

    oracle();
    return checks_done("s02-e02-my-memset-memcmp-memchr");
}
