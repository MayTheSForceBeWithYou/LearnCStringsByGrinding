/* s00-r03-escape-reps: harness. Do not edit; write your code in esc.c.
 *
 * The course's own renderer, lib/bytes.h, follows the same contract, so it is the
 * oracle here: your esc_buf must agree with bytes_notation() byte for byte, for every
 * capacity from 0 to one more than the full rendering needs. */
#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "course.h"
#include "esc.h"
#include "oracle.h"
#include "strcheck.h"

static void one_case(unsigned char b, const char *want)
{
    size_t need = strlen(want);
    char *out = guard_new(need + 1);

    check(esc_one(out, need + 1, b) == need, "esc_one returns the length it needs");
    check_str(out, want, "esc_one renders the byte");
    check_guard(out, "esc_one wrote nothing outside the buffer");
    guard_free(out);
}

/* The same rendering through every capacity: too small, exact, roomy. */
static void capacities(const void *p, size_t n, const char *want)
{
    size_t need = strlen(want);

    for (size_t cap = 0; cap <= need + 2; cap++) {
        char *out = guard_new(cap);
        size_t got = esc_buf(out, cap, p, n);
        char expect[64];

        check(got == need, "esc_buf returns the full length whatever the capacity");
        if (cap > 0) {
            size_t keep = need < cap - 1 ? need : cap - 1;
            memcpy(expect, want, keep);
            expect[keep] = '\0';
            check_str(out, expect, "esc_buf fills the buffer as far as it goes and terminates");
        }
        check_guard(out, "esc_buf wrote nothing outside the buffer");
        guard_free(out);
    }
}

static void oracle(void)
{
    enum { MAXLEN = 12, CASES = 1500 };
    char src[MAXLEN + 1];
    char mine[128], theirs[128];
    int ok = 1;

    orc_seed(3);
    for (int i = 0; i < CASES && ok; i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_HIGH);
        char *only = guard_copy(src, len + 1);
        size_t a = esc_buf(mine, sizeof mine, only, len + 1);
        size_t b = bytes_notation(only, len + 1, theirs, sizeof theirs);

        if (a != b || strcmp(mine, theirs) != 0) {
            ok = 0;
            printf("       seed 3, case %u, %zu bytes\n", (unsigned)orc_case(), len + 1);
            printf("       yours: %s (returned %zu)\n", mine, a);
            printf("       lib:   %s (returned %zu)\n", theirs, b);
        }
        guard_free(only);
    }
    check(ok, "esc_buf agrees with lib/bytes.h on 1500 generated buffers");
}

int main(void)
{
    check(esc_hex_digit(0) == '0', "esc_hex_digit(0)");
    check(esc_hex_digit(9) == '9', "esc_hex_digit(9)");
    check(esc_hex_digit(10) == 'A', "esc_hex_digit(10) is upper case");
    check(esc_hex_digit(15) == 'F', "esc_hex_digit(15)");
    check(esc_hex_digit(16) == -1, "esc_hex_digit(16) is an error");
    check(esc_hex_digit(255) == -1, "esc_hex_digit(255) is an error");

    one_case('a', "a");
    one_case(' ', " ");
    one_case('~', "~");
    one_case('\0', "\\0");
    one_case('\n', "\\n");
    one_case('\t', "\\t");
    one_case('\r', "\\r");
    one_case('\\', "\\\\");
    one_case(0x1F, "\\x1F");
    one_case(0x7F, "\\x7F");
    one_case(0x80, "\\x80");
    one_case(0xE9, "\\xE9");
    one_case(0xFF, "\\xFF");

    capacities("hi", 3, "hi\\0");
    capacities("a\nb", 4, "a\\nb\\0");
    capacities("\xE9\x80", 2, "\\xE9\\x80");
    capacities("", 0, "");

    {   /* esc_str shows the terminator; esc_buf over the same bytes agrees. */
        char *out = guard_new(16);
        check(esc_str(out, 16, "hi") == 4, "esc_str returns the length including the \\0");
        check_str(out, "hi\\0", "esc_str renders the string and its terminator");
        check(esc_str(out, 16, "") == 2, "esc_str of the empty string is two characters");
        check_str(out, "\\0", "esc_str of the empty string is \\0");
        check(esc_str(out, 3, "hello") == 7, "esc_str returns the full length when cut short");
        check_str(out, "he", "esc_str cut to two characters and a terminator");
        guard_free(out);
    }

    oracle();
    return checks_done("s00-r03-escape-reps");
}
