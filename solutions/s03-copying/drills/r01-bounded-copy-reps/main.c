/* s03-r01-bounded-copy-reps: harness. Do not edit; write your code in bcopy.c. */
#include <stdio.h>
#include <string.h>

#include "bcopy.h"
#include "course.h"
#include "strcheck.h"

static const struct {
    const char *src;
    size_t cap;
    const char *want; /* the string dst must hold after a truncating copy */
} cases[] = {
    {"hello", 8, "hello"},       /* fits */
    {"abcdefg", 8, "abcdefg"},   /* exact fit */
    {"abcdefgh", 8, "abcdefg"},  /* one too many */
    {"abcdefghijklmnop", 8, "abcdefg"},
    {"", 8, ""},
    {"", 1, ""},
    {"x", 1, ""},                /* room for the terminator only */
    {"xy", 2, "x"},
    {"caf\xC3\xA9", 5, "caf\xC3"}, /* a byte copy knows nothing about characters */
};

int main(void)
{
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        const char *src = cases[i].src, *want = cases[i].want;
        size_t cap = cases[i].cap, len = strlen(src);
        int cut = len >= cap;
        char *only = guard_copy(src, len + 1);
        char *dst = guard_new(cap);

        check(bc_loop(dst, cap, only) == len, "bc_loop returns strlen(src)");
        check_str(dst, want, "bc_loop stores the string, cut to cap - 1 characters");
        check_guard(dst, "bc_loop stays inside cap bytes");

        guard_reset(dst);
        check(bc_format(dst, cap, only) == cut, "bc_format returns 1 exactly when truncated");
        check_str(dst, want, "bc_format stores the string, cut to cap - 1 characters");
        check_guard(dst, "bc_format stays inside cap bytes");

        guard_reset(dst);
        check(bc_measured(dst, cap, only) == cut, "bc_measured returns 1 exactly when truncated");
        check_str(dst, want, "bc_measured stores the string, cut to cap - 1 characters");
        check_guard(dst, "bc_measured stays inside cap bytes");

        guard_reset(dst);
        check(bc_strict(dst, cap, only) == (cut ? -1 : 0), "bc_strict returns -1 exactly when src does not fit");
        check_str(dst, cut ? "" : src, "bc_strict stores all of src, or the empty string");
        check_guard(dst, "bc_strict stays inside cap bytes");

        guard_free(dst);
        guard_free(only);
    }

    /* None of the four pads: bytes after the terminator are left alone. */
    char *dst = guard_new(8);
    bc_loop(dst, 8, "ab");
    check_bytes(dst, "ab\0#####", 8, "bc_loop does not pad");
    guard_reset(dst);
    bc_format(dst, 8, "ab");
    check_bytes(dst, "ab\0#####", 8, "bc_format does not pad");
    guard_reset(dst);
    bc_measured(dst, 8, "ab");
    check_bytes(dst, "ab\0#####", 8, "bc_measured does not pad");
    guard_reset(dst);
    bc_strict(dst, 8, "ab");
    check_bytes(dst, "ab\0#####", 8, "bc_strict does not pad");
    guard_free(dst);

    return checks_done("s03-r01-bounded-copy-reps");
}
