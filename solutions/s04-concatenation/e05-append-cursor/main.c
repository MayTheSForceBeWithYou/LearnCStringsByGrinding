/* s04-e05-append-cursor: harness. Do not edit; write your code in cursor.c.
 *
 * Every buffer is exactly `cap` bytes, so a write at buf[cap] is a fence failure in
 * the plain build and a heap-buffer-overflow under ASan.
 */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "cursor.h"
#include "strcheck.h"

int main(void)
{
    /* The ordinary sequence: three appends that fit. */
    {
        char *buf = guard_new(16);
        Cursor c;

        cur_init(&c, buf, 16);
        check_str(buf, "", "cur_init makes the buffer the empty string");
        check(cur_len(&c) == 0, "and its length 0");
        check(!cur_truncated(&c), "and nothing is truncated yet");
        cur_puts(&c, "ab");
        cur_putc(&c, '/');
        cur_puts(&c, "cd");
        check_str(buf, "ab/cd", "three appends in order");
        check(cur_len(&c) == 5, "cur_len counts the characters");
        check(!cur_truncated(&c), "nothing was dropped");
        check(strlen(buf) == cur_len(&c), "cur_len agrees with strlen");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* An exact fit, then one character too many. */
    {
        char *buf = guard_new(4);
        Cursor c;

        cur_init(&c, buf, 4);
        cur_puts(&c, "abc");
        check_str(buf, "abc", "an append that exactly fills the buffer");
        check(!cur_truncated(&c), "an exact fit is not a truncation");
        cur_putc(&c, 'd');
        check_str(buf, "abc", "a character that does not fit is dropped");
        check(cur_truncated(&c), "and the cursor says so");
        check(cur_len(&c) == 3, "and the length is unchanged");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* Truncation is sticky: later appends that fit do not clear it. */
    {
        char *buf = guard_new(8);
        Cursor c;

        cur_init(&c, buf, 8);
        cur_puts(&c, "abcdefghij");
        check_str(buf, "abcdefg", "cur_puts stores what fits");
        check(cur_truncated(&c), "and reports the truncation");
        cur_init(&c, buf, 8);
        cur_puts(&c, "ab");
        check(!cur_truncated(&c), "cur_init clears the flag");
        cur_puts(&c, "cdefghij");
        check(cur_truncated(&c), "truncated again");
        cur_puts(&c, "");
        cur_putc(&c, 'x');
        check(cur_truncated(&c), "and the flag is sticky");
        check_str(buf, "abcdefg", "the buffer still holds what fitted");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* cur_printf, including a format whose output does not fit. */
    {
        char *buf = guard_new(16);
        Cursor c;

        cur_init(&c, buf, 16);
        cur_printf(&c, "%s=%d", "n", 42);
        check_str(buf, "n=42", "cur_printf formats and appends");
        check(cur_len(&c) == 4, "and keeps the length");
        cur_printf(&c, " %s", "ok");
        check_str(buf, "n=42 ok", "a second cur_printf continues where the first stopped");
        check(!cur_truncated(&c), "nothing dropped yet");
        cur_printf(&c, " %s", "0123456789");
        check_str(buf, "n=42 ok 0123456", "a cur_printf that does not fit stores what does");
        check(cur_truncated(&c), "and reports it");
        check(cur_len(&c) == 15, "the buffer is full: cap - 1 characters");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* cap == 1: only the terminator fits, so every append is a truncation. */
    {
        char *buf = guard_new(1);
        Cursor c;

        cur_init(&c, buf, 1);
        check_str(buf, "", "cap 1 is the empty string");
        cur_puts(&c, "a");
        check_str(buf, "", "nothing fits in cap 1");
        check(cur_truncated(&c), "and that is a truncation");
        cur_putc(&c, 'b');
        cur_printf(&c, "%d", 7);
        check_str(buf, "", "still nothing");
        check(cur_len(&c) == 0, "and the length is still 0");
        check_guard(buf, "nothing written outside the buffer");
        guard_free(buf);
    }

    /* cap == 0: there is no byte to write, not even a terminator. */
    {
        char *buf = guard_new(0);
        Cursor c;

        cur_init(&c, buf, 0);
        check(cur_len(&c) == 0, "cap 0 has length 0");
        cur_puts(&c, "abc");
        cur_putc(&c, 'x');
        cur_printf(&c, "%d", 7);
        check(cur_truncated(&c), "every append into cap 0 is a truncation");
        check(cur_len(&c) == 0, "and the length stays 0");
        check_guard(buf, "and no byte is written");
        guard_free(buf);
    }

    /* Appending nothing, and appending bytes above 0x7F. */
    {
        char *buf = guard_new(8);
        Cursor c;

        cur_init(&c, buf, 8);
        cur_puts(&c, "");
        check(!cur_truncated(&c) && cur_len(&c) == 0, "appending the empty string changes nothing");
        cur_puts(&c, "caf\xC3\xA9");
        check_str(buf, "caf\xC3\xA9", "bytes above 0x7F are ordinary characters");
        check(cur_len(&c) == 5, "and count as bytes");
        guard_free(buf);
    }

    /* A cursor over a buffer the caller already uses: the appender must not assume
     * the buffer starts empty beyond what cur_init does. */
    {
        char *buf = guard_new(8);
        Cursor c;

        memcpy(buf, "ZZZZZZZ", 8);
        cur_init(&c, buf, 8);
        check_str(buf, "", "cur_init overwrites whatever was there");
        cur_puts(&c, "ab");
        check_bytes(buf, "ab\0ZZZZ", 8, "and only the bytes up to the terminator are written");
        guard_free(buf);
    }

    return checks_done("s04-e05-append-cursor");
}
