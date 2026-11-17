/* s01-e02-fixed-fields: harness. Do not edit; write your code in field.c.
 *
 * Every field is a buffer of exactly `width` bytes, so reading field[width] -- which
 * is what strlen would do to a full field -- is a sanitizer report here.
 */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "field.h"
#include "strcheck.h"

/* The records of a small fixed-width format: an 8-byte name field each. */
#define W 8
static const char names[][W] = {
    {'a', 'd', 'a', 0, 0, 0, 0, 0},                    /* "ada", padded          */
    {'g', 'r', 'a', 'c', 'e', 0, 0, 0},                /* "grace", padded        */
    {'m', 'a', 'x', 'i', 'm', 'i', 'l', 'i'},          /* 8 characters: full     */
    {0, 0, 0, 0, 0, 0, 0, 0},                          /* empty                  */
    {'a', 'd', 'a', 0, 'x', 'x', 'x', 'x'},            /* "ada", then junk       */
};

static void len_case(const char *bytes, size_t width, size_t want, const char *msg)
{
    char *only = guard_copy(bytes, width);

    check(field_len(only, width) == want, msg);
    check_guard(only, "field_len wrote nothing");
    guard_free(only);
}

static void cstr_case(const char *bytes, size_t width, size_t cap, size_t want_ret,
                      const char *want_bytes, const char *msg)
{
    char *only = guard_copy(bytes, width);
    char *dst = guard_new(cap);

    check(field_to_cstr(dst, cap, only, width) == want_ret, msg);
    check_bytes(dst, want_bytes, cap, msg);
    check_guard(dst, "field_to_cstr wrote nothing outside dst");
    check_guard(only, "field_to_cstr wrote nothing in the field");
    guard_free(dst);
    guard_free(only);
}

int main(void)
{
    /* field_len over the five records. */
    len_case(names[0], W, 3, "field_len of a padded field");
    len_case(names[1], W, 5, "field_len of a longer padded field");
    len_case(names[2], W, 8, "field_len of a full field is the width");
    len_case(names[3], W, 0, "field_len of an empty field is 0");
    len_case(names[4], W, 3, "field_len stops at the first NUL, whatever follows it");
    len_case("A123", 4, 4, "field_len of a four-character field of width 4");
    len_case("\xFF\xE9\x80", 3, 3, "field_len counts bytes above 0x7F");
    len_case("", 0, 0, "field_len with width 0 is 0 and reads nothing");

    /* field_eq: the padding is not part of the text. */
    {
        char *a = guard_copy(names[0], W);
        char *b = guard_copy(names[4], W);
        char *c = guard_copy(names[1], W);
        char *full = guard_copy(names[2], W);
        char *empty = guard_copy(names[3], W);

        check(field_eq(a, a, W) == 1, "a field equals itself");
        check(field_eq(a, b, W) == 1, "two fields with the same text and different padding are equal");
        check(field_eq(a, c, W) == 0, "fields with different text are not equal");
        check(field_eq(c, a, W) == 0, "and the other way round");
        check(field_eq(empty, empty, W) == 1, "two empty fields are equal");
        check(field_eq(a, empty, W) == 0, "a field is not equal to an empty one");
        check(field_eq(full, full, W) == 1, "two full fields with the same text are equal");
        check(field_eq(full, a, W) == 0, "a full field is not equal to a shorter one");
        check(field_eq(a, b, 0) == 1, "with width 0 every pair of fields is equal");
        guard_free(empty);
        guard_free(full);
        guard_free(c);
        guard_free(b);
        guard_free(a);
    }

    /* A full field of width 2 against a padded one that starts with the same bytes. */
    {
        char *x = guard_copy("ab", 2);
        char *y = guard_copy("a\0", 2);

        check(field_eq(x, y, 2) == 0, "\"ab\" and \"a\" are different text in a 2-byte field");
        check(field_len(x, 2) == 2 && field_len(y, 2) == 1, "and their lengths say so");
        guard_free(y);
        guard_free(x);
    }

    /* field_to_cstr: the snprintf contract over a field. */
    cstr_case(names[0], W, 8, 3, "ada\0####", "field_to_cstr of a padded field");
    cstr_case(names[0], W, 4, 3, "ada\0", "field_to_cstr into a buffer of the exact size");
    cstr_case(names[0], W, 3, 3, "ad\0", "field_to_cstr truncates and still reports 3");
    cstr_case(names[0], W, 1, 3, "\0", "field_to_cstr with cap 1 writes only the terminator");
    cstr_case(names[0], W, 0, 3, "", "field_to_cstr with cap 0 writes nothing");
    cstr_case(names[2], W, 9, 8, "maximili\0", "field_to_cstr terminates a full field");
    cstr_case(names[2], W, 5, 8, "maxi\0", "field_to_cstr truncates a full field and reports 8");
    cstr_case(names[3], W, 4, 0, "\0###", "field_to_cstr of an empty field");
    cstr_case(names[4], W, 8, 3, "ada\0####", "field_to_cstr copies the text, not the junk");
    cstr_case("A123", 4, 5, 4, "A123\0", "field_to_cstr of a field with no terminator at all");

    /* The three together, the way a record reader uses them. */
    {
        char *f = guard_copy(names[2], W);
        char out[W + 1];
        size_t n = field_to_cstr(out, sizeof out, f, W);

        check(n == 8 && n < sizeof out, "a width-8 field always fits in 9 bytes");
        check(strlen(out) == field_len(f, W), "the copy's strlen is the field's length");
        check(field_eq(f, f, W), "and the field is unchanged");
        guard_free(f);
    }

    return checks_done("s01-e02-fixed-fields");
}
