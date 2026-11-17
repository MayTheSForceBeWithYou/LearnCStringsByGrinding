/* cursor.h -- s04-e05-append-cursor: an allocation-free bounded appender.
 *
 * A Cursor remembers one buffer, its capacity, how much of it is used, and whether
 * anything has ever been dropped. Appending is then a call that cannot overflow and
 * cannot forget to terminate, and the caller checks for truncation once, at the end,
 * instead of after every append.
 *
 * Truncation is STICKY: once something has not fitted, cur_truncated() stays true for
 * the life of the cursor, however many successful appends follow. That is what makes
 * one check at the end enough.
 */
#ifndef CURSOR_H
#define CURSOR_H

#include <stdarg.h>
#include <stddef.h>

typedef struct {
    char *buf;       /* the caller's buffer                                   */
    size_t cap;      /* its size in bytes, terminator included                */
    size_t len;      /* characters written so far (never more than cap - 1)   */
    int truncated;   /* 1 once anything has been dropped                      */
} Cursor;

/* Prepares a cursor over buf, which has cap bytes. If cap > 0 the buffer becomes the
 * empty string at once, so it is a string even if nothing is ever appended. cap may
 * be 0, and then nothing is ever written and every append is a truncation. */
void cur_init(Cursor *c, char *buf, size_t cap);

/* Appends the string s, or as much of it as fits. The buffer is terminated afterwards
 * whenever cap > 0. Sets the truncated flag if any character was dropped. */
void cur_puts(Cursor *c, const char *s);

/* Appends one character, or sets the truncated flag if it does not fit. A '\0' passed
 * here is appended as an ordinary byte, which makes the buffer hold an embedded NUL;
 * the cursor's own len is unaffected by that. */
void cur_putc(Cursor *c, char ch);

/* Appends printf-formatted text, or as much of it as fits, using vsnprintf. Sets the
 * truncated flag if anything was dropped or if the formatting failed. */
void cur_printf(Cursor *c, const char *fmt, ...);

/* 1 if anything has ever been dropped from this cursor, else 0. */
int cur_truncated(const Cursor *c);

/* The number of characters in the buffer: what strlen would say, without the walk. */
size_t cur_len(const Cursor *c);

#endif
