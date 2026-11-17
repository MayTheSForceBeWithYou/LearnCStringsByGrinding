/* s01-e01-my-strlen: harness. Do not edit; write your code in mylen.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "mylen.h"
#include "oracle.h"
#include "strcheck.h"

/* A string in a buffer of exactly its own size: one byte past the terminator is a
 * sanitizer report, not a lucky read. */
static void len_case(const char *s)
{
    size_t want = strlen(s);
    char *only = guard_copy(s, want + 1);

    check(my_strlen(only) == want, "my_strlen agrees with strlen");
    check(my_strnlen(only, want + 1) == want, "my_strnlen with maxlen one more than the length");
    check(my_strnlen(only, want) == want, "my_strnlen with maxlen equal to the length");
    check(my_strnlen(only, 0) == 0, "my_strnlen with maxlen 0 is 0 and reads nothing");
    guard_free(only);
}

/* A character sequence with no terminator anywhere: my_strnlen must return width and
 * must not read the byte after it. my_strlen is not called here, because it could not
 * answer without reading past the end. */
static void field_case(const char *bytes, size_t width)
{
    char *only = guard_copy(bytes, width);

    check(my_strnlen(only, width) == width, "my_strnlen of a full field is the field width");
    for (size_t m = 0; m < width; m++)
        check(my_strnlen(only, m) == m, "my_strnlen of a full field is maxlen for every smaller maxlen");
    check_guard(only, "my_strnlen wrote nothing");
    guard_free(only);
}

static void oracle(void)
{
    enum { MAXLEN = 48, CASES = 3000 };
    char src[MAXLEN + 1];
    int len_ok = 1, nlen_ok = 1;

    orc_seed(101);
    for (int i = 0; i < CASES && (len_ok || nlen_ok); i++) {
        size_t want = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, want + 1);
        size_t maxlen = (size_t)orc_below((unsigned)want + 3);

        if (my_strlen(only) != want) {
            len_ok = 0;
            printf("       my_strlen differs from strlen: seed 101, case %u, s: %s\n",
                   (unsigned)orc_case(), bytes_show(only, want + 1));
            printf("       yours: %zu   libc: %zu\n", my_strlen(only), want);
        }
        if (my_strnlen(only, maxlen) != strnlen(only, maxlen)) {
            nlen_ok = 0;
            printf("       my_strnlen differs from strnlen: seed 101, case %u, maxlen %zu, s: %s\n",
                   (unsigned)orc_case(), maxlen, bytes_show(only, want + 1));
            printf("       yours: %zu   libc: %zu\n", my_strnlen(only, maxlen), strnlen(only, maxlen));
        }
        guard_free(only);
    }
    check(len_ok, "my_strlen agrees with strlen on 3000 generated strings");
    check(nlen_ok, "my_strnlen agrees with strnlen on 3000 generated strings and maxlens");
}

int main(void)
{
    len_case("");
    len_case("x");
    len_case("hello");
    len_case("a longer string, with spaces and 1234567890");
    len_case("caf\xC3\xA9 \xFF\x80"); /* bytes above 0x7F are ordinary characters */

    /* An embedded NUL ends the string; the bytes after it are not part of it. */
    {
        char *only = guard_copy("ab\0cd", 6);
        check(my_strlen(only) == 2, "my_strlen stops at an embedded NUL");
        check(my_strnlen(only, 6) == 2, "my_strnlen stops at an embedded NUL");
        check(my_strnlen(only, 1) == 1, "my_strnlen with maxlen inside the first field");
        guard_free(only);
    }

    field_case("A123", 4);
    field_case("\xFF\xE9\x80", 3);
    field_case("x", 1);

    {   /* maxlen 0 on a buffer with no readable byte at all. */
        char *none = guard_new(0);
        check(my_strnlen(none, 0) == 0, "my_strnlen(s, 0) reads nothing, even with no bytes to read");
        guard_free(none);
    }

    oracle();
    return checks_done("s01-e01-my-strlen");
}
