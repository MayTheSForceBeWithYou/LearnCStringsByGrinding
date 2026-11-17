/* s01-r02-end-pointer-reps: harness. Do not edit; write your code in endreps.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "endreps.h"
#include "oracle.h"
#include "strcheck.h"

static void one(const char *s)
{
    size_t len = strlen(s);
    char *only = guard_copy(s, len + 1);

    check(end_of(only) == only + len, "end_of points at the terminator");
    check(*end_of(only) == '\0', "and the byte there is a NUL");
    check(len_between(only, end_of(only)) == len, "len_between(s, end_of(s)) is the length");
    if (len == 0)
        check(last_char(only) == NULL, "last_char of the empty string is NULL");
    else
        check(last_char(only) == only + len - 1, "last_char points at the last character");
    check_guard(only, "nothing was written");
    guard_free(only);
}

static void oracle(void)
{
    enum { MAXLEN = 40, CASES = 2000 };
    char src[MAXLEN + 1];
    int ok = 1;

    orc_seed(13);
    for (int i = 0; i < CASES && ok; i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, len + 1);
        char *want_last = len == 0 ? NULL : only + len - 1;

        if (end_of(only) != only + len || last_char(only) != want_last ||
            len_between(only, only + len) != len) {
            ok = 0;
            printf("       seed 13, case %u, s: %s\n", (unsigned)orc_case(), bytes_show(only, len + 1));
            printf("       end_of s + %td (want s + %zu), len_between %zu\n",
                   end_of(only) - only, len, len_between(only, only + len));
        }
        guard_free(only);
    }
    check(ok, "end_of, last_char and len_between agree on 2000 generated strings");
}

int main(void)
{
    one("");
    one("x");
    one("hello");
    one("a longer string, with spaces and 1234567890");
    one("caf\xC3\xA9 \xFF\x80");

    /* len_between over a range that is not a whole string. */
    {
        char *s = guard_copy("hello", 6);
        check(len_between(s, s) == 0, "len_between of one point is 0");
        check(len_between(s, s + 1) == 1, "len_between of neighbouring bytes is 1");
        check(len_between(s + 1, s + 4) == 3, "len_between inside the string");
        check(len_between(s, s + 6) == 6, "len_between over the whole buffer");
        guard_free(s);
    }

    /* room_left: the appender's question. A 8-byte buffer, filled step by step. */
    {
        char *buf = guard_new(8);
        const char *buf_end = buf + 8;

        memcpy(buf, "abc\0####", 8);
        check(room_left(buf, buf_end) == 7, "an empty 8-byte buffer has room for 7 characters");
        check(room_left(buf + 3, buf_end) == 4, "after 3 characters there is room for 4 more");
        check(room_left(buf + 7, buf_end) == 0, "at the last byte there is room for none");
        check(room_left(buf_end, buf_end) == 0, "at the end of the buffer there is room for none");
        check(room_left(end_of(buf), buf_end) == 4, "room_left after end_of: the appender's pair");
        guard_free(buf);
    }

    /* The pair that builds a string: write while there is room, then terminate. */
    {
        char *buf = guard_new(8);
        const char *buf_end = buf + 8;
        char *p = buf;
        const char *src = "abcdefghij";

        buf[0] = '\0';
        for (size_t i = 0; src[i] != '\0' && room_left(p, buf_end) > 0; i++)
            *p++ = src[i];
        *p = '\0';
        check_bytes(buf, "abcdefg\0", 8, "writing while room_left > 0 fills the buffer and terminates");
        check(len_between(buf, p) == 7, "and len_between says how much was written");
        guard_free(buf);
    }

    oracle();
    return checks_done("s01-r02-end-pointer-reps");
}
