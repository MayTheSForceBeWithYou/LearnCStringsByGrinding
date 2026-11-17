/* s00-e02-count-and-terminate: harness. Do not edit; write your code in count.c. */
#include <stdio.h>
#include <string.h>

#include "count.h"
#include "course.h"
#include "oracle.h"
#include "strcheck.h"

/* Every source is a buffer of exactly strlen(s) + 1 bytes, so a read past the
 * terminator is a report rather than a lucky success. */
static void len_case(const char *s)
{
    size_t want = strlen(s);
    char *only = guard_copy(s, want + 1);

    check(str_len(only) == want, "str_len counts the bytes before the terminator");
    check(str_end(only) == only + want, "str_end points at the terminator itself");
    check(*str_end(only) == '\0', "the byte str_end points at is a NUL");
    check(buf_is_terminated(only, want + 1), "a string of exactly its own size is terminated");
    guard_free(only);
}

static void oracle(void)
{
    enum { MAXLEN = 48, CASES = 3000 };
    char src[MAXLEN + 1];
    int len_ok = 1, end_ok = 1;

    orc_seed(1);
    for (int i = 0; i < CASES && (len_ok || end_ok); i++) {
        size_t want = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, want + 1);
        size_t got = str_len(only);
        const char *e = str_end(only);

        if (got != want) {
            len_ok = 0;
            printf("       str_len differs from strlen: seed 1, case %u, s: %s\n",
                   (unsigned)orc_case(), bytes_show(only, want + 1));
            printf("       yours: %zu   libc: %zu\n", got, want);
        }
        if (e != only + want) {
            end_ok = 0;
            printf("       str_end is wrong: seed 1, case %u, s: %s\n",
                   (unsigned)orc_case(), bytes_show(only, want + 1));
            printf("       yours: s + %td   expected: s + %zu\n", e - only, want);
        }
        guard_free(only);
    }
    check(len_ok, "str_len agrees with strlen on 3000 generated strings");
    check(end_ok, "str_end agrees with s + strlen(s) on 3000 generated strings");
}

/* The bytes at buf are data of exactly cap bytes: there is nothing after them to read. */
static void term_case(const char *bytes, size_t cap, int want, const char *msg)
{
    char *only = guard_copy(bytes, cap);
    check(buf_is_terminated(only, cap) == want, msg);
    check_guard(only, "buf_is_terminated wrote nothing");
    guard_free(only);
}

static void set_case(size_t cap, char c, size_t n, const char *want, size_t want_ret,
                     const char *msg)
{
    char *dst = guard_new(cap);
    size_t got = buf_set(dst, cap, c, n);

    check(got == want_ret, msg);
    check_bytes(dst, want, cap, msg);
    check_guard(dst, "buf_set wrote nothing outside the buffer");
    guard_free(dst);
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
        char *only = guard_copy("a\0bc", 5);
        check(str_len(only) == 1, "str_len stops at an embedded NUL");
        check(str_end(only) == only + 1, "str_end stops at an embedded NUL");
        guard_free(only);
    }

    term_case("", 1, 1, "one byte, a NUL: terminated");
    term_case("hi", 3, 1, "a terminator at the last byte: terminated");
    term_case("hi\0junk", 7, 1, "a terminator in the middle: terminated");
    term_case("abc", 3, 0, "three characters and no NUL in three bytes: not terminated");
    term_case("abcdefghij", 10, 0, "ten characters and no NUL in ten bytes: not terminated");
    term_case("\xFF\xE9\x80", 3, 0, "bytes above 0x7F are not terminators");
    term_case("x", 0, 0, "cap == 0: not terminated, and no byte is read");

    set_case(8, 'x', 3, "xxx\0####", 3, "buf_set writes n copies and a terminator");
    set_case(8, 'x', 7, "xxxxxxx\0", 7, "buf_set with n == cap - 1 fills the buffer exactly");
    set_case(8, 'x', 8, "xxxxxxx\0", 7, "buf_set with n == cap truncates to cap - 1");
    set_case(8, 'x', 99, "xxxxxxx\0", 7, "buf_set with n far too large truncates to cap - 1");
    set_case(8, 'x', 0, "\0#######", 0, "buf_set with n == 0 writes only the terminator");
    set_case(1, 'x', 5, "\0", 0, "buf_set with cap == 1 writes only the terminator");
    set_case(8, '\xE9', 2, "\xE9\xE9\0#####", 2, "buf_set stores a byte above 0x7F unchanged");
    set_case(8, '\0', 3, "\0\0\0\0####", 3, "buf_set with c == '\\0' writes NUL bytes and a terminator");
    set_case(8, 'y', (size_t)-1, "yyyyyyy\0", 7, "buf_set with n == SIZE_MAX truncates to cap - 1");

    {   /* cap == 0: nothing may be written, not even a terminator. */
        char *dst = guard_new(0);
        check(buf_set(dst, 0, 'x', 4) == 0, "buf_set with cap == 0 returns 0");
        check_guard(dst, "buf_set with cap == 0 writes nothing");
        guard_free(dst);
    }

    {   /* What buf_set leaves behind is a string, and it is the string you asked for. */
        char *dst = guard_new(16);
        size_t wrote = buf_set(dst, 16, '=', 40);
        check(wrote == 15, "buf_set reports 15 copies in a 16-byte buffer");
        check(buf_is_terminated(dst, 16), "the buffer buf_set filled is terminated");
        check(str_len(dst) == 15, "str_len of a full buf_set result is cap - 1");
        check(str_end(dst) == dst + 15, "str_end of a full buf_set result is dst + cap - 1");
        guard_free(dst);
    }

    oracle();
    return checks_done("s00-e02-count-and-terminate");
}
