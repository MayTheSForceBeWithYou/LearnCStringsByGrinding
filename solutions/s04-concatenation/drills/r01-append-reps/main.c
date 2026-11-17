/* s04-r01-append-reps: harness. Do not edit; write your code in app.c. */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "course.h"
#include "strcheck.h"

int main(void)
{
    /* app_room is arithmetic on two pointers and reads no memory. */
    {
        char *buf = guard_new(8);
        char *end = buf + 8;

        check(app_room(buf, end) == 7, "an empty 8-byte buffer has room for 7 characters");
        check(app_room(buf + 3, end) == 4, "three characters in, there is room for 4");
        check(app_room(buf + 7, end) == 0, "at the last byte there is room for none");
        check(app_room(end, end) == 0, "at the end there is room for none");
        check_guard(buf, "app_room writes nothing");
        guard_free(buf);
    }

    /* The ordinary chain: three appends that fit. */
    {
        char *buf = guard_new(16);
        char *end = buf + 16;
        char *p = buf;

        *p = '\0';
        p = app_str(p, end, "id=");
        p = app_dec(p, end, 42);
        p = app_char(p, end, ';');
        check_str(buf, "id=42;", "three chained appends");
        check(p == buf + 6, "and p is at the terminator");
        check(app_room(p, end) == 9, "with room for 9 more");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* An exact fit, then one character too many. */
    {
        char *buf = guard_new(4);
        char *end = buf + 4;
        char *p = buf;

        *p = '\0';
        p = app_str(p, end, "abc");
        check_str(buf, "abc", "an append that exactly fills the buffer");
        check(p == buf + 3 && app_room(p, end) == 0, "and leaves no room");
        p = app_char(p, end, 'd');
        check_str(buf, "abc", "a character that does not fit is dropped");
        check(p == end - 1, "and p stays at the last writable position");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* app_str truncating. */
    {
        char *buf = guard_new(8);
        char *end = buf + 8;
        char *p = buf;

        *p = '\0';
        p = app_str(p, end, "abcdefghij");
        check_str(buf, "abcdefg", "app_str stores what fits");
        check(p == end - 1, "and returns the last writable position");
        p = app_str(p, end, "xyz");
        check_str(buf, "abcdefg", "a later append adds nothing");
        guard_free(buf);
    }

    /* app_dec: the digits, the sign, the extremes. */
    {
        char *buf = guard_new(32);
        char *end = buf + 32;

        buf[0] = '\0';
        check(app_dec(buf, end, 0) == buf + 1, "0 is one character");
        check_str(buf, "0", "0");
        buf[0] = '\0';
        app_dec(buf, end, 7);
        check_str(buf, "7", "7");
        buf[0] = '\0';
        app_dec(buf, end, 1234567890);
        check_str(buf, "1234567890", "a ten-digit number");
        buf[0] = '\0';
        app_dec(buf, end, -42);
        check_str(buf, "-42", "a negative number");
        buf[0] = '\0';
        app_dec(buf, end, LONG_MIN);
        {
            char want[32];

            snprintf(want, sizeof want, "%ld", LONG_MIN);
            check_str(buf, want, "LONG_MIN, which cannot be negated");
        }
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* app_dec truncating: what fits of the digits, then a terminator. */
    {
        char *buf = guard_new(4);
        char *end = buf + 4;

        buf[0] = '\0';
        check(app_dec(buf, end, 12345) == end - 1, "a number that does not fit returns end - 1");
        check_str(buf, "123", "and stores the leading digits");
        buf[0] = '\0';
        app_dec(buf, end, -1234);
        check_str(buf, "-12", "a negative number keeps its sign and loses digits");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* A one-byte buffer: only the terminator fits. */
    {
        char *buf = guard_new(1);
        char *end = buf + 1;
        char *p = buf;

        *p = '\0';
        p = app_str(p, end, "abc");
        p = app_char(p, end, 'x');
        p = app_dec(p, end, 9);
        check_str(buf, "", "nothing fits in a one-byte buffer");
        check(p == buf, "and p never moves");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* Bytes above 0x7F are ordinary characters. */
    {
        char *buf = guard_new(8);
        char *end = buf + 8;
        char *p = buf;

        *p = '\0';
        p = app_str(p, end, "caf\xC3\xA9");
        check_str(buf, "caf\xC3\xA9", "bytes above 0x7F");
        check(p == buf + 5, "counted as bytes");
        guard_free(buf);
    }

    return checks_done("s04-r01-append-reps");
}
