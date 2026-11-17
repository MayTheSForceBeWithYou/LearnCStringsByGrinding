/* s03-e06-copy-policy: harness. Do not edit; write your code in copypol.c. */
#include <stdio.h>
#include <string.h>

#include "copypol.h"
#include "course.h"
#include "strcheck.h"

/* The source is handed over in a buffer that ends at its terminator. */
static int run(int (*fn)(char *, size_t, const char *), char *dst, size_t cap, const char *src)
{
    char *only = guard_copy(src, strlen(src) + 1);
    int rc = fn(dst, cap, only);
    guard_free(only);
    return rc;
}

static void test_trunc(void)
{
    char *dst = guard_new(8);
    char *none = guard_new(0);

    check(run(copy_trunc, dst, 8, "hello") == COPY_OK, "copy_trunc: a string that fits is COPY_OK");
    check_str(dst, "hello", "copy_trunc: a string that fits is stored whole");
    guard_reset(dst);
    check(run(copy_trunc, dst, 8, "abcdefg") == COPY_OK, "copy_trunc: 7 characters fit in 8 bytes");
    check_bytes(dst, "abcdefg\0", 8, "copy_trunc: exact fit");
    guard_reset(dst);
    check(run(copy_trunc, dst, 8, "abcdefgh") == COPY_TRUNCATED, "copy_trunc: 8 characters in 8 bytes is COPY_TRUNCATED");
    check_bytes(dst, "abcdefg\0", 8, "copy_trunc: truncated to cap - 1 characters and terminated");
    guard_reset(dst);
    check(run(copy_trunc, dst, 8, "a much longer source string") == COPY_TRUNCATED, "copy_trunc: a long source is COPY_TRUNCATED");
    check_bytes(dst, "a much \0", 8, "copy_trunc: a long source is cut and terminated");
    guard_reset(dst);
    check(run(copy_trunc, dst, 8, "") == COPY_OK, "copy_trunc: the empty string fits");
    check(dst[0] == '\0', "copy_trunc: the empty string is stored as a terminator");
    guard_reset(dst);
    check(run(copy_trunc, dst, 1, "x") == COPY_TRUNCATED, "copy_trunc: cap 1 holds only the empty string");
    check_bytes(dst, "\0#######", 8, "copy_trunc: cap 1 writes one byte");
    check(run(copy_trunc, none, 0, "x") == COPY_NO_ROOM, "copy_trunc: cap 0 is COPY_NO_ROOM");
    check(run(copy_trunc, none, 0, "") == COPY_NO_ROOM, "copy_trunc: cap 0 is COPY_NO_ROOM even for the empty string");
    check_guard(dst, "copy_trunc: nothing written outside the buffer");
    check_guard(none, "copy_trunc: a zero-capacity buffer is not touched");
    guard_free(none);
    guard_free(dst);
}

static void test_strict(void)
{
    char *dst = guard_new(8);
    char *none = guard_new(0);

    check(run(copy_strict, dst, 8, "hello") == COPY_OK, "copy_strict: a string that fits is COPY_OK");
    check_str(dst, "hello", "copy_strict: a string that fits is stored whole");
    check(run(copy_strict, dst, 8, "abcdefg") == COPY_OK, "copy_strict: 7 characters fit in 8 bytes");
    check_bytes(dst, "abcdefg\0", 8, "copy_strict: exact fit");
    check(run(copy_strict, dst, 8, "abcdefgh") == COPY_NO_ROOM, "copy_strict: 8 characters in 8 bytes is COPY_NO_ROOM");
    check(dst[0] == '\0', "copy_strict: on COPY_NO_ROOM dst is the empty string, not the old value");
    check(run(copy_strict, dst, 8, "ok") == COPY_OK, "copy_strict: usable again after a failure");
    check_str(dst, "ok", "copy_strict: stores \"ok\" after a failure");
    check(run(copy_strict, dst, 8, "a much longer source string") == COPY_NO_ROOM, "copy_strict: a long source is COPY_NO_ROOM");
    check(dst[0] == '\0', "copy_strict: a long source leaves the empty string");
    check(run(copy_strict, dst, 8, "") == COPY_OK, "copy_strict: the empty string fits");
    check(dst[0] == '\0', "copy_strict: the empty string is stored");
    guard_reset(dst);
    check(run(copy_strict, dst, 1, "") == COPY_OK, "copy_strict: the empty string fits in 1 byte");
    check(run(copy_strict, dst, 1, "x") == COPY_NO_ROOM, "copy_strict: one character does not fit in 1 byte");
    check_bytes(dst, "\0#######", 8, "copy_strict: cap 1 writes one byte at most");
    check(run(copy_strict, none, 0, "") == COPY_NO_ROOM, "copy_strict: cap 0 is COPY_NO_ROOM");
    check_guard(dst, "copy_strict: nothing written outside the buffer");
    check_guard(none, "copy_strict: a zero-capacity buffer is not touched");
    guard_free(none);
    guard_free(dst);
}

static void test_n(void)
{
    char *dst = guard_new(8);
    char *none = guard_new(0);
    /* Sources with NO terminator: exactly n readable bytes. */
    char *five = guard_copy("hello", 5);
    char *seven = guard_copy("abcdefg", 7);
    char *eight = guard_copy("abcdefgh", 8);
    char *zero = guard_new(0);
    char *inner = guard_copy("ab\0cd", 5);

    check(copy_n(dst, 8, five, 5) == COPY_OK, "copy_n: 5 characters fit in 8 bytes");
    check_bytes(dst, "hello\0##", 8, "copy_n: n characters and a terminator, nothing more");
    guard_reset(dst);
    check(copy_n(dst, 8, five, 3) == COPY_OK, "copy_n: a prefix of the source");
    check_bytes(dst, "hel\0####", 8, "copy_n: copies exactly n characters");
    guard_reset(dst);
    check(copy_n(dst, 8, seven, 7) == COPY_OK, "copy_n: 7 characters fit in 8 bytes");
    check_bytes(dst, "abcdefg\0", 8, "copy_n: exact fit");
    check(copy_n(dst, 8, eight, 8) == COPY_NO_ROOM, "copy_n: 8 characters in 8 bytes is COPY_NO_ROOM");
    check(dst[0] == '\0', "copy_n: on COPY_NO_ROOM dst is the empty string");
    guard_reset(dst);
    check(copy_n(dst, 8, zero, 0) == COPY_OK, "copy_n: n == 0 stores the empty string");
    check_bytes(dst, "\0#######", 8, "copy_n: n == 0 writes one terminator and reads nothing");
    guard_reset(dst);
    check(copy_n(dst, 8, inner, 5) == COPY_OK, "copy_n: a NUL among the n bytes is copied like any byte");
    check_bytes(dst, "ab\0cd\0##", 8, "copy_n: all n bytes are copied, then the terminator");
    check(copy_n(none, 0, five, 5) == COPY_NO_ROOM, "copy_n: cap 0 is COPY_NO_ROOM");
    check(copy_n(none, 0, zero, 0) == COPY_NO_ROOM, "copy_n: cap 0 is COPY_NO_ROOM even for n == 0");
    check(copy_n(dst, 8, five, (size_t)-1) == COPY_NO_ROOM, "copy_n: a huge n is COPY_NO_ROOM, without overflow");
    check_guard(dst, "copy_n: nothing written outside the buffer");
    check_guard(none, "copy_n: a zero-capacity buffer is not touched");

    guard_free(inner);
    guard_free(zero);
    guard_free(eight);
    guard_free(seven);
    guard_free(five);
    guard_free(none);
    guard_free(dst);
}

int main(void)
{
    test_trunc();
    test_strict();
    test_n();
    return checks_done("s03-e06-copy-policy");
}
