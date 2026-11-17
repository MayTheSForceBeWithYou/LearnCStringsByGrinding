/* s03-r02-fixed-field-reps: harness. Do not edit; write your code in field.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "field.h"
#include "strcheck.h"

int main(void)
{
    enum { W = 8 };
    char *f = guard_new(W); /* exactly 8 bytes: touching f[8] is an error */
    char *out = guard_new(16);

    check(field_put(f, W, "tar") == 0, "field_put: a short value is not cut");
    check_bytes(f, "tar\0\0\0\0\0", W, "field_put pads a short value with NULs to the width");
    check(field_len(f, W) == 3, "field_len of a short value");
    check(field_eq(f, W, "tar") == 1, "field_eq: equal");
    check(field_eq(f, W, "ta") == 0, "field_eq: the field is longer than s");
    check(field_eq(f, W, "tart") == 0, "field_eq: s is longer than the field's value");
    check(field_eq(f, W, "tap") == 0, "field_eq: same length, different character");
    check(field_get(out, 16, f, W) == 3, "field_get returns the field's length");
    check_bytes(out, "tar\0############", 16, "field_get writes the value and one terminator");

    guard_reset(f);
    check(field_put(f, W, "abcdefgh") == 0, "field_put: a value of exactly the width is not cut");
    check_bytes(f, "abcdefgh", W, "field_put: a full field has no terminator");
    check(field_len(f, W) == 8, "field_len of a full field is the width");
    check(field_eq(f, W, "abcdefgh") == 1, "field_eq: a full field equals its 8-character value");
    check(field_eq(f, W, "abcdefg") == 0, "field_eq: a full field is not equal to its 7-character prefix");
    check(field_eq(f, W, "abcdefghi") == 0, "field_eq: a full field is not equal to a longer string");
    guard_reset(out);
    check(field_get(out, 16, f, W) == 8, "field_get on a full field returns the width");
    check_bytes(out, "abcdefgh\0#######", 16, "field_get terminates what the field does not");
    guard_reset(out);
    check(field_get(out, 5, f, W) == 8, "field_get into a small buffer still returns the field's length");
    check_bytes(out, "abcd\0###########", 16, "field_get cuts to cap - 1 characters and terminates");
    guard_reset(out);
    check(field_get(out, 1, f, W) == 8, "field_get with cap 1");
    check_bytes(out, "\0###############", 16, "field_get with cap 1 writes only the terminator");

    guard_reset(f);
    check(field_put(f, W, "abcdefghij") == 1, "field_put: a longer value is cut and reported");
    check_bytes(f, "abcdefgh", W, "field_put keeps the first width characters");

    guard_reset(f);
    check(field_put(f, W, "") == 0, "field_put: the empty string");
    check_bytes(f, "\0\0\0\0\0\0\0\0", W, "field_put: an empty value is all NULs");
    check(field_len(f, W) == 0, "field_len of an empty field");
    check(field_eq(f, W, "") == 1, "field_eq: an empty field equals \"\"");
    check(field_eq(f, W, "a") == 0, "field_eq: an empty field is not \"a\"");
    guard_reset(out);
    check(field_get(out, 16, f, W) == 0, "field_get on an empty field returns 0");
    check(out[0] == '\0', "field_get on an empty field stores the empty string");

    /* Overwriting a long value with a short one must not leave the old tail behind. */
    field_put(f, W, "longname");
    field_put(f, W, "ab");
    check_bytes(f, "ab\0\0\0\0\0\0", W, "field_put over an old value leaves no stale characters");
    check(field_eq(f, W, "ab") == 1, "field_eq after overwriting");

    check_guard(f, "nothing written outside the 8-byte field");
    check_guard(out, "nothing written outside the output buffer");
    guard_free(out);
    guard_free(f);
    return checks_done("s03-r02-fixed-field-reps");
}
