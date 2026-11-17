/* s02-e01-my-memcpy-memmove: harness. Do not edit; write your code in mymove.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mymove.h"
#include "oracle.h"
#include "strcheck.h"

/* A disjoint copy: the destination is exactly n bytes, the source exactly n bytes, so
 * one byte too many in either direction is a report. */
static void copy_case(const char *bytes, size_t n, const char *msg)
{
    char *src = guard_copy(bytes, n);
    char *dst = guard_new(n);

    check(my_memcpy(dst, src, n) == dst, "my_memcpy returns dst");
    check_bytes(dst, bytes, n, msg);
    check_bytes(src, bytes, n, "my_memcpy leaves the source alone");
    check_guard(dst, "my_memcpy writes exactly n bytes");
    guard_free(dst);
    guard_free(src);
}

/* Every overlap there is: dst at a fixed place, src from 32 bytes before it to 32
 * bytes after it, compared with libc's memmove on the same bytes. */
static void overlap_sweep(size_t n)
{
    enum { PAD = 40 };
    const size_t size = PAD + n + PAD;
    char *mine = guard_new(size);
    char ref[256];
    int ok = 1;

    for (int d = -32; d <= 32 && ok; d++) {
        for (size_t i = 0; i < size; i++) {
            char c = (char)('a' + (i % 26));
            mine[i] = c;
            ref[i] = c;
        }
        char *dst = mine + PAD;
        char *src = mine + PAD + d;
        char *r = my_memmove(dst, src, n);

        memmove(ref + PAD, ref + PAD + d, n);
        if (r != dst || memcmp(mine, ref, size) != 0) {
            ok = 0;
            printf("       my_memmove differs from memmove: n = %zu, src = dst %+d\n", n, d);
            printf("       yours: %s\n", bytes_show(mine + PAD - 4, n + 8));
            printf("       libc:  %s\n", bytes_show(ref + PAD - 4, n + 8));
        }
    }
    check(ok, "my_memmove agrees with memmove at every overlap from -32 to +32");
    check_guard(mine, "my_memmove writes nothing outside the destination");
    guard_free(mine);
}

static void oracle(void)
{
    enum { MAXLEN = 48, CASES = 2000 };
    char src[MAXLEN + 1], ref[MAXLEN + 1];
    int cpy_ok = 1, mov_ok = 1;

    orc_seed(201);
    for (int i = 0; i < CASES && (cpy_ok || mov_ok); i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        size_t n = (size_t)orc_below((unsigned)len + 1);
        char *only = guard_copy(src, len + 1);
        char *dst = guard_new(len + 1);

        memset(ref, GUARD_FILL, sizeof ref);
        memcpy(ref, src, n);
        guard_reset(dst);
        if (my_memcpy(dst, only, n) != dst || memcmp(dst, ref, len + 1) != 0) {
            cpy_ok = 0;
            printf("       my_memcpy differs: seed 201, case %u, n = %zu\n", (unsigned)orc_case(), n);
            printf("       yours: %s\n", bytes_show(dst, len + 1));
            printf("       libc:  %s\n", bytes_show(ref, len + 1));
        }
        guard_reset(dst);
        memset(ref, GUARD_FILL, sizeof ref);
        memmove(ref, src, n);
        if (my_memmove(dst, only, n) != dst || memcmp(dst, ref, len + 1) != 0) {
            mov_ok = 0;
            printf("       my_memmove differs on disjoint buffers: seed 201, case %u, n = %zu\n",
                   (unsigned)orc_case(), n);
        }
        guard_free(dst);
        guard_free(only);
    }
    check(cpy_ok, "my_memcpy agrees with memcpy on 2000 generated buffers");
    check(mov_ok, "my_memmove agrees with memmove on 2000 generated buffers");
}

int main(void)
{
    copy_case("hello", 5, "my_memcpy copies five bytes");
    copy_case("a", 1, "my_memcpy copies one byte");
    copy_case("", 0, "my_memcpy with n == 0 copies nothing");
    copy_case("ab\0cd", 5, "my_memcpy copies a NUL like any other byte");
    copy_case("\xFF\x00\x80\xE9", 4, "my_memcpy copies bytes above 0x7F");
    copy_case("0123456789abcdefghijklmnopqrstuvwxyz", 36, "my_memcpy copies a longer buffer");

    /* n == 0 must not touch either buffer, whatever the pointers are. */
    {
        char *dst = guard_new(0);
        char *src = guard_new(0);

        check(my_memcpy(dst, src, 0) == dst, "my_memcpy with n == 0 returns dst");
        check(my_memmove(dst, src, 0) == dst, "my_memmove with n == 0 returns dst");
        guard_free(src);
        guard_free(dst);
    }

    /* dst == src is an overlap too, and must leave the bytes as they were. */
    {
        char *buf = guard_copy("abcdefgh", 8);

        check(my_memmove(buf, buf, 8) == buf, "my_memmove with dst == src returns dst");
        check_bytes(buf, "abcdefgh", 8, "my_memmove with dst == src changes nothing");
        guard_free(buf);
    }

    overlap_sweep(1);
    overlap_sweep(8);
    overlap_sweep(24);
    overlap_sweep(33);

    oracle();
    return checks_done("s02-e01-my-memcpy-memmove");
}
