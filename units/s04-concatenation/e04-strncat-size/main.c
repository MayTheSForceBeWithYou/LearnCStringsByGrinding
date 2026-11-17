/* s04-e04-strncat-size: harness. The defect is in label.c, not here.
 *
 * Every destination is a buffer of exactly `cap` bytes, so a write at dst[cap] is a
 * fence failure in the plain build and a heap-buffer-overflow under ASan.
 */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "label.h"
#include "strcheck.h"

static void append_case(const char *before, size_t cap, const char *src, int want_rc,
                        const char *want, const char *msg)
{
    char *dst = guard_new(cap);

    memcpy(dst, before, strlen(before) + 1);
    check(label_append(dst, cap, src) == want_rc, msg);
    check_str(dst, want, msg);
    check_guard(dst, "label_append writes nothing outside the buffer");
    guard_free(dst);
}

static void build_case(size_t cap, const char *kind, const char *text, int want_rc,
                       const char *want, const char *msg)
{
    char *dst = guard_new(cap);

    check(label_build(dst, cap, kind, text) == want_rc, msg);
    check_str(dst, want, msg);
    check_guard(dst, "label_build writes nothing outside the buffer");
    guard_free(dst);
}

int main(void)
{
    /* Appends that fit. */
    append_case("ab", 8, "cd", 0, "abcd", "an append that fits");
    append_case("", 8, "cd", 0, "cd", "an append onto an empty string");
    append_case("ab", 8, "", 0, "ab", "an append of nothing");
    append_case("abcdef", 8, "g", 0, "abcdefg", "an append that exactly fills the buffer");

    /* Appends that do not fit: truncate, report -1, and stay inside the buffer. */
    append_case("abcdef", 8, "gh", -1, "abcdefg", "an append of one byte too many");
    append_case("ab", 8, "cdefghij", -1, "abcdefg", "an append far too long");
    append_case("abcdefg", 8, "h", -1, "abcdefg", "an append onto a full buffer");

    /* The whole label. */
    build_case(16, "warn", "disk", 0, "warn: disk", "a label that fits");
    build_case(11, "warn", "disk", 0, "warn: disk", "a label that exactly fits");
    build_case(10, "warn", "disk", -1, "warn: dis", "a label one byte too long");
    build_case(8, "warn", "disk", -1, "warn: d", "a label that is cut in the text");
    build_case(5, "warn", "disk", -1, "warn", "a label with no room for the separator");
    build_case(3, "warn", "disk", -1, "wa", "a label cut inside the kind");
    build_case(1, "warn", "disk", -1, "", "a label with room only for the terminator");

    return checks_done("s04-e04-strncat-size");
}
