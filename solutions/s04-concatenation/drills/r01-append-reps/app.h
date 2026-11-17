/* app.h -- s04-r01-append-reps: appending at an end pointer.
 *
 * The state of an append is two pointers: `p`, where the next character goes, and
 * `end`, one past the last byte of the buffer. Every function here takes both, writes
 * what fits, terminates, and returns the new `p` -- so appends chain without ever
 * measuring what is already there.
 *
 * In all four: p <= end, and the buffer has at least one byte (end > start), so there
 * is always room for a terminator. A function that cannot fit everything writes what
 * it can, terminates, and returns `end - 1`, which is the position from which nothing
 * more will ever fit.
 */
#ifndef APP_H
#define APP_H

#include <stddef.h>

/* How many more characters fit at p: (end - p) - 1, and 0 when p is at end. */
size_t app_room(const char *p, const char *end);

/* Appends the string s, or as much of it as fits, and terminates. Returns the new p. */
char *app_str(char *p, char *end, const char *s);

/* Appends one character if it fits, and terminates. Returns the new p. */
char *app_char(char *p, char *end, char c);

/* Appends the decimal form of v (which may be negative), or as much of it as fits,
 * and terminates. Returns the new p. The digits are produced by hand: no snprintf. */
char *app_dec(char *p, char *end, long v);

#endif
