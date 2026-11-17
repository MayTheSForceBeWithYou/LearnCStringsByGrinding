/* s02-r01-gap-reps: harness. Do not edit; write your code in gap.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "gap.h"
#include "strcheck.h"

enum { CAP = 12 };

/* A CAP-byte buffer whose first `len` bytes are the given text and whose remaining
 * bytes are '#', so that a move which touches them is visible. */
static char *fresh(const char *text, size_t *len)
{
    char *buf = guard_new(CAP);

    *len = strlen(text);
    memcpy(buf, text, *len);
    return buf;
}

static void open_case(const char *text, size_t at, size_t n, size_t want_len,
                      const char *want, const char *msg)
{
    size_t len;
    char *buf = fresh(text, &len);

    check(gap_open(buf, CAP, len, at, n) == want_len, msg);
    check_bytes(buf, want, CAP, msg);
    check_guard(buf, "gap_open writes nothing outside the buffer");
    guard_free(buf);
}

static void close_case(const char *text, size_t at, size_t n, size_t want_len,
                       const char *want, const char *msg)
{
    size_t len;
    char *buf = fresh(text, &len);

    check(gap_close(buf, CAP, len, at, n) == want_len, msg);
    check_bytes(buf, want, CAP, msg);
    check_guard(buf, "gap_close writes nothing outside the buffer");
    guard_free(buf);
}

static void delete_case(const char *text, size_t from, size_t to, size_t want_len,
                        const char *want, const char *msg)
{
    size_t len;
    char *buf = fresh(text, &len);

    check(gap_delete(buf, CAP, len, from, to) == want_len, msg);
    check_bytes(buf, want, CAP, msg);
    guard_free(buf);
}

static void insert_case(const char *text, size_t at, const char *src, size_t want_len,
                        const char *want, const char *msg)
{
    size_t len, n = strlen(src);
    char *buf = fresh(text, &len);
    char *s = guard_copy(src, n);

    check(gap_insert(buf, CAP, len, at, s, n) == want_len, msg);
    check_bytes(buf, want, CAP, msg);
    check_guard(buf, "gap_insert writes nothing outside the buffer");
    guard_free(s);
    guard_free(buf);
}

int main(void)
{
    /* gap_open: the bytes move up, the gap keeps what was there. */
    open_case("abcdef", 3, 2, 8, "abcdedef####", "gap_open in the middle");
    open_case("abcdef", 0, 2, 8, "ababcdef####", "gap_open at the front");
    open_case("abcdef", 6, 2, 8, "abcdef######", "gap_open at the end moves nothing");
    open_case("abcdef", 3, 0, 6, "abcdef######", "gap_open of nothing");
    open_case("abcdef", 3, 6, 12, "abcdef###def", "gap_open that exactly fills the buffer");
    open_case("abcdef", 3, 7, 6, "abcdef######", "gap_open that does not fit does nothing");
    open_case("abcdef", 7, 2, 6, "abcdef######", "gap_open past the end does nothing");

    /* gap_close: the tail moves down, the bytes at the end are left behind. */
    close_case("abcdef", 2, 2, 4, "abefef######", "gap_close in the middle");
    close_case("abcdef", 0, 2, 4, "cdefef######", "gap_close at the front");
    close_case("abcdef", 4, 2, 4, "abcdef######", "gap_close at the end moves nothing");
    close_case("abcdef", 2, 0, 6, "abcdef######", "gap_close of nothing");
    close_case("abcdef", 0, 6, 0, "abcdef######", "gap_close of everything");
    close_case("abcdef", 4, 3, 6, "abcdef######", "gap_close past the end does nothing");

    /* gap_delete: a range, which is the same move said differently. */
    delete_case("abcdef", 1, 3, 4, "adefef######", "gap_delete of a middle range");
    delete_case("abcdef", 0, 0, 6, "abcdef######", "gap_delete of an empty range");
    delete_case("abcdef", 0, 6, 0, "abcdef######", "gap_delete of everything");
    delete_case("abcdef", 5, 6, 5, "abcdef######", "gap_delete of the last byte");
    delete_case("abcdef", 3, 2, 6, "abcdef######", "gap_delete with from > to does nothing");
    delete_case("abcdef", 4, 9, 6, "abcdef######", "gap_delete past the end does nothing");

    /* gap_insert: open a gap and fill it. */
    insert_case("abcdef", 3, "XY", 8, "abcXYdef####", "gap_insert in the middle");
    insert_case("abcdef", 0, "XY", 8, "XYabcdef####", "gap_insert at the front");
    insert_case("abcdef", 6, "XY", 8, "abcdefXY####", "gap_insert at the end");
    insert_case("abcdef", 3, "", 6, "abcdef######", "gap_insert of nothing");
    insert_case("abcdef", 2, "XYZXYZ", 12, "abXYZXYZcdef", "gap_insert that exactly fills");
    insert_case("abcdef", 2, "XYZXYZX", 6, "abcdef######", "gap_insert that does not fit");

    return checks_done("s02-r01-gap-reps");
}
