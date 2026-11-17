/* s03-e01-my-strcpy-stpcpy: harness. Do not edit; write your code in mycopy.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mycopy.h"
#include "oracle.h"
#include "strcheck.h"

/* One fixed case, three ways: into a buffer of exactly the right size (one byte more
 * would be an overflow), into a roomier buffer (the spare bytes must stay untouched),
 * and from a source that ends at its terminator (reading past it is an over-read). */
static void fixed_case(const char *src)
{
    size_t size = strlen(src) + 1;
    char *exact = guard_new(size);
    char *roomy = guard_new(size + 4);
    char *only = guard_copy(src, size);
    char want[64];

    char *r = my_strcpy(exact, only);
    check(r == exact, "my_strcpy returns dst");
    check_bytes(exact, src, size, "my_strcpy copies the characters and the terminator");
    check_guard(exact, "my_strcpy writes exactly strlen(src) + 1 bytes");

    memset(want, GUARD_FILL, sizeof want);
    memcpy(want, src, size);
    r = my_strcpy(roomy, only);
    check(r == roomy, "my_strcpy returns dst (roomy buffer)");
    check_bytes(roomy, want, size + 4, "my_strcpy leaves the bytes after the terminator alone");

    guard_reset(exact);
    r = my_stpcpy(exact, only);
    check(r == exact + size - 1, "my_stpcpy returns a pointer to the terminator it wrote");
    check_bytes(exact, src, size, "my_stpcpy copies the characters and the terminator");
    check_guard(exact, "my_stpcpy writes exactly strlen(src) + 1 bytes");

    guard_reset(roomy);
    r = my_stpcpy(roomy, only);
    check(r == roomy + size - 1, "my_stpcpy returns dst + strlen(src) (roomy buffer)");
    check_bytes(roomy, want, size + 4, "my_stpcpy leaves the bytes after the terminator alone");

    guard_free(only);
    guard_free(roomy);
    guard_free(exact);
}

/* Differential test: the same generated strings through libc and through yours. */
static void oracle(void)
{
    enum { MAXLEN = 40, CAP = MAXLEN + 8, CASES = 3000 };
    char src[MAXLEN + 1], ref[CAP];
    char *dst = guard_new(CAP);
    int strcpy_ok = 1, stpcpy_ok = 1;

    orc_seed(301);
    for (int i = 0; i < CASES && (strcpy_ok || stpcpy_ok); i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);

        if (strcpy_ok) {
            memset(ref, GUARD_FILL, CAP);
            strcpy(ref, src);
            guard_reset(dst);
            char *r = my_strcpy(dst, src);
            if (r != dst || memcmp(dst, ref, CAP) != 0) {
                strcpy_ok = 0;
                printf("       my_strcpy differs from strcpy: seed 301, case %u, src (length %zu): %s\n",
                       (unsigned)orc_case(), len, bytes_show(src, len + 1));
                printf("       yours: %s\n", bytes_show(dst, CAP));
                printf("       libc:  %s\n", bytes_show(ref, CAP));
            }
        }
        if (stpcpy_ok) {
            memset(ref, GUARD_FILL, CAP);
            char *e = stpcpy(ref, src);
            guard_reset(dst);
            char *r = my_stpcpy(dst, src);
            if (r - dst != e - ref || memcmp(dst, ref, CAP) != 0) {
                stpcpy_ok = 0;
                printf("       my_stpcpy differs from stpcpy: seed 301, case %u, src (length %zu): %s\n",
                       (unsigned)orc_case(), len, bytes_show(src, len + 1));
                printf("       yours: %s  returned dst + %td\n", bytes_show(dst, CAP), r - dst);
                printf("       libc:  %s  returned dst + %td\n", bytes_show(ref, CAP), e - ref);
            }
        }
    }
    check(strcpy_ok, "my_strcpy agrees with strcpy on 3000 generated strings");
    check(stpcpy_ok, "my_stpcpy agrees with stpcpy on 3000 generated strings");
    check_guard(dst, "nothing written outside the destination during the generated cases");
    guard_free(dst);
}

int main(void)
{
    fixed_case("hello");
    fixed_case("");
    fixed_case("x");
    fixed_case("a longer string, with spaces");
    fixed_case("caf\xC3\xA9 \xFF\x80"); /* bytes above 0x7F are ordinary characters */

    /* Chaining: the reason stpcpy exists. */
    char *buf = guard_new(12);
    char *end = my_stpcpy(buf, "usr");
    end = my_stpcpy(end, "/");
    end = my_stpcpy(end, "bin");
    check(end == buf + 7, "three chained my_stpcpy calls end at the terminator");
    check_bytes(buf, "usr/bin\0####", 12, "three chained my_stpcpy calls build \"usr/bin\"");
    guard_free(buf);

    oracle();
    return checks_done("s03-e01-my-strcpy-stpcpy");
}
