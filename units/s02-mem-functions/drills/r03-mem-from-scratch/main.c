/* s02-r03-mem-from-scratch: harness. Do not edit; write your code in scratch.c.
 *
 * Every one of the five is compared with the libc function of the same name, on fixed
 * cases and on generated ones, with every buffer sized exactly to what the call may
 * touch. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "oracle.h"
#include "scratch.h"
#include "strcheck.h"

static void copy_case(const char *bytes, size_t n, const char *msg)
{
    char *src = guard_copy(bytes, n);
    char *dst = guard_new(n);

    check(s_memcpy(dst, src, n) == dst, "s_memcpy returns dst");
    check_bytes(dst, bytes, n, msg);
    check_guard(dst, "s_memcpy writes exactly n bytes");
    guard_free(dst);
    guard_free(src);
}

static void overlap_sweep(size_t n)
{
    enum { PAD = 40 };
    const size_t size = PAD + n + PAD;
    char *mine = guard_new(size);
    char ref[128];
    int ok = 1;

    for (int d = -16; d <= 16 && ok; d++) {
        for (size_t i = 0; i < size; i++) {
            char c = (char)('a' + (i % 26));
            mine[i] = c;
            ref[i] = c;
        }
        char *dst = mine + PAD;
        char *r = s_memmove(dst, mine + PAD + d, n);

        memmove(ref + PAD, ref + PAD + d, n);
        if (r != dst || memcmp(mine, ref, size) != 0) {
            ok = 0;
            printf("       s_memmove differs: n = %zu, src = dst %+d\n", n, d);
            printf("       yours: %s\n", bytes_show(mine + PAD - 2, n + 4));
            printf("       libc:  %s\n", bytes_show(ref + PAD - 2, n + 4));
        }
    }
    check(ok, "s_memmove agrees with memmove at every overlap from -16 to +16");
    guard_free(mine);
}

static void oracle(void)
{
    enum { MAXLEN = 24, CASES = 2000 };
    char a[MAXLEN + 1], b[MAXLEN + 1], ref[MAXLEN + 1];
    int ok = 1;

    orc_seed(213);
    for (int i = 0; i < CASES && ok; i++) {
        size_t la = orc_string(a, MAXLEN, ORC_ALPHA_AB);
        size_t lb = orc_string(b, MAXLEN, ORC_ALPHA_AB);
        size_t n = la < lb ? la : lb;
        int c = (int)(unsigned char)"ab\xE9"[orc_below(3)];
        char *pa = guard_copy(a, n);
        char *pb = guard_copy(b, n);
        char *dst = guard_new(n);

        memcpy(ref, a, n);
        if (s_memcpy(dst, pa, n) != dst || memcmp(dst, ref, n) != 0 ||
            sign_of(s_memcmp(pa, pb, n)) != sign_of(memcmp(pa, pb, n)) ||
            s_memchr(pa, c, n) != memchr(pa, c, n)) {
            ok = 0;
            printf("       seed 213, case %u, n = %zu, c = %s\n",
                   (unsigned)orc_case(), n, bytes_show(&c, 1));
            printf("       a: %s\n       b: %s\n", bytes_show(pa, n), bytes_show(pb, n));
        }
        guard_reset(dst);
        memset(ref, c, n);
        s_memset(dst, c, n);
        if (memcmp(dst, ref, n) != 0) {
            ok = 0;
            printf("       s_memset differs: seed 213, case %u, n = %zu\n", (unsigned)orc_case(), n);
        }
        guard_free(dst);
        guard_free(pb);
        guard_free(pa);
    }
    check(ok, "all five agree with libc on 2000 generated buffers");
}

int main(void)
{
    copy_case("hello", 5, "s_memcpy copies five bytes");
    copy_case("", 0, "s_memcpy with n == 0 copies nothing");
    copy_case("a\0b\xFF", 4, "s_memcpy copies NUL and high bytes");

    overlap_sweep(1);
    overlap_sweep(12);

    {   /* s_memset: the value is converted to unsigned char. */
        char *buf = guard_new(8);

        check(s_memset(buf, 'x', 3) == buf, "s_memset returns s");
        check_bytes(buf, "xxx#####", 8, "s_memset writes n bytes");
        guard_reset(buf);
        s_memset(buf, 0x141, 4);
        check_bytes(buf, "AAAA####", 8, "s_memset converts c to unsigned char");
        guard_reset(buf);
        s_memset(buf, 'z', 0);
        check_bytes(buf, "########", 8, "s_memset with n == 0 writes nothing");
        check_guard(buf, "s_memset writes nothing outside the buffer");
        guard_free(buf);
    }

    {   /* s_memcmp: sign only, unsigned char. */
        char *a = guard_copy("\x7F", 1);
        char *b = guard_copy("\x80", 1);

        check(sign_of(s_memcmp(a, b, 1)) == -1, "0x7F is smaller than 0x80");
        check(sign_of(s_memcmp(b, a, 1)) == 1, "and the other way round");
        check(s_memcmp(a, b, 0) == 0, "n == 0 compares equal");
        guard_free(b);
        guard_free(a);
    }

    {   /* s_memchr: NULL when absent, NUL found like any byte. */
        char *s = guard_copy("ab\0cd", 5);

        check(s_memchr(s, 'a', 5) == s, "s_memchr finds the first byte");
        check(s_memchr(s, 0, 5) == s + 2, "s_memchr finds a NUL byte");
        check(s_memchr(s, 'd', 5) == s + 4, "s_memchr looks past a NUL byte");
        check(s_memchr(s, 'z', 5) == NULL, "s_memchr returns NULL when absent");
        check(s_memchr(s, 'd', 3) == NULL, "s_memchr respects n");
        check(s_memchr(s, 'a', 0) == NULL, "s_memchr with n == 0 is NULL");
        guard_free(s);
    }

    oracle();
    return checks_done("s02-r03-mem-from-scratch");
}
