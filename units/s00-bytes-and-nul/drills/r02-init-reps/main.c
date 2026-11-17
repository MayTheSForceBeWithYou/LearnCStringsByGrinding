/* s00-r02-init-reps: harness. Do not edit; write your code in init.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "init.h"
#include "strcheck.h"

/* Every destination is a buffer of exactly cap bytes, pre-filled with '#', so an
 * unwritten byte, a stray terminator and a write past the end are all visible. */
static void empty_case(size_t cap, int want, const char *bytes, const char *msg)
{
    char *buf = guard_new(cap);

    check(init_empty(buf, cap) == want, msg);
    check_bytes(buf, bytes, cap, msg);
    check_guard(buf, "init_empty wrote nothing outside the buffer");
    guard_free(buf);
}

static void repeat_case(size_t cap, char c, size_t n, size_t want, const char *bytes,
                        const char *msg)
{
    char *buf = guard_new(cap);

    check(init_repeat(buf, cap, c, n) == want, msg);
    check_bytes(buf, bytes, cap, msg);
    check_guard(buf, "init_repeat wrote nothing outside the buffer");
    guard_free(buf);
}

/* lit is held in a buffer of exactly its own size: reading one byte past its
 * terminator is a report, and so is reading the byte after a source that was cut. */
static void literal_case(size_t cap, const char *lit, size_t want, const char *bytes,
                         const char *msg)
{
    char *buf = guard_new(cap);
    char *src = guard_copy(lit, strlen(lit) + 1);

    check(init_literal(buf, cap, src) == want, msg);
    check_bytes(buf, bytes, cap, msg);
    check_guard(buf, "init_literal wrote nothing outside the buffer");
    guard_free(src);
    guard_free(buf);
}

static void term_case(const char *bytes, size_t cap, int want, const char *msg)
{
    char *buf = guard_copy(bytes, cap);

    check(init_terminated(buf, cap) == want, msg);
    check_guard(buf, "init_terminated wrote nothing");
    guard_free(buf);
}

int main(void)
{
    empty_case(8, 0, "\0#######", "init_empty writes one terminator");
    empty_case(1, 0, "\0", "init_empty with cap == 1");
    empty_case(0, -1, "", "init_empty with cap == 0 writes nothing and returns -1");

    repeat_case(8, '-', 3, 3, "---\0####", "init_repeat writes n copies and a terminator");
    repeat_case(8, '-', 7, 7, "-------\0", "init_repeat with n == cap - 1 fills the buffer");
    repeat_case(8, '-', 8, 7, "-------\0", "init_repeat with n == cap truncates");
    repeat_case(8, '-', 0, 0, "\0#######", "init_repeat with n == 0 writes only the terminator");
    repeat_case(1, '-', 9, 0, "\0", "init_repeat with cap == 1 writes only the terminator");
    repeat_case(0, '-', 9, 0, "", "init_repeat with cap == 0 writes nothing");
    repeat_case(8, '\xE9', 2, 2, "\xE9\xE9\0#####", "init_repeat stores a byte above 0x7F");
    repeat_case(8, 'z', (size_t)-1, 7, "zzzzzzz\0", "init_repeat with n == SIZE_MAX truncates");

    literal_case(8, "abc", 3, "abc\0####", "init_literal copies a string that fits");
    literal_case(8, "abcdefg", 7, "abcdefg\0", "init_literal copies a string that fits exactly");
    literal_case(8, "abcdefgh", 7, "abcdefg\0", "init_literal truncates a string one byte too long");
    literal_case(8, "abcdefghijklmno", 7, "abcdefg\0", "init_literal truncates a long string");
    literal_case(8, "", 0, "\0#######", "init_literal of the empty string writes a terminator");
    literal_case(1, "abc", 0, "\0", "init_literal with cap == 1 writes only the terminator");
    literal_case(0, "abc", 0, "", "init_literal with cap == 0 writes nothing");
    literal_case(8, "caf\xC3\xA9", 5, "caf\xC3\xA9\0##", "init_literal copies bytes above 0x7F");

    term_case("", 1, 1, "one byte, a NUL: terminated");
    term_case("hi", 3, 1, "a terminator at the last byte: terminated");
    term_case("hi\0junk", 7, 1, "a terminator in the middle: terminated");
    term_case("abc", 3, 0, "three characters and no NUL in three bytes: not terminated");
    term_case("\xFF\xE9\x80", 3, 0, "bytes above 0x7F are not terminators");
    term_case("x", 0, 0, "cap == 0: not terminated, and no byte is read");

    /* The four together: whatever the first three do, the result is a string. */
    {
        char *buf = guard_new(6);

        check(init_empty(buf, 6) == 0 && init_terminated(buf, 6), "init_empty leaves a string");
        check(init_repeat(buf, 6, 'x', 99) == 5 && init_terminated(buf, 6),
              "a truncating init_repeat leaves a string");
        check_str(buf, "xxxxx", "and that string is five x");
        check(init_literal(buf, 6, "much too long") == 5 && init_terminated(buf, 6),
              "a truncating init_literal leaves a string");
        check_str(buf, "much ", "and that string is the first five characters");
        guard_free(buf);
    }

    return checks_done("s00-r02-init-reps");
}
