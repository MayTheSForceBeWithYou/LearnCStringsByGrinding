/* count.h -- s00-e02-count-and-terminate: the terminator convention, by hand.
 *
 * Four functions that do what the library does, with no library call at all. They are
 * the convention written out: a string is bytes followed by a NUL, a buffer is bytes
 * and a capacity, and the two are not the same thing.
 */
#ifndef COUNT_H
#define COUNT_H

#include <stddef.h>

/* The length of the string s: how many bytes come before its terminator.
 *
 * Requires: s is a string. Every byte from s[0] up to and including the first NUL is
 *           readable. (A buffer with no NUL in it is not a string, and asking this
 *           question about one reads past the object.)
 * Reads:    exactly strlen(s) + 1 bytes, and no byte after the terminator.
 * Returns:  the number of bytes before the terminator. 0 for the empty string.
 * Written:  by index: s[0], s[1], ... with a subscript, not a moving pointer. */
size_t str_len(const char *s);

/* A pointer to the terminator of the string s.
 *
 * Requires, reads: exactly as str_len.
 * Returns:  &s[str_len(s)]: the address of the NUL byte itself, never NULL. For the
 *           empty string that is s. The difference str_end(s) - s is the length.
 * Written:  by pointer: one pointer that moves, no subscript and no counter. */
const char *str_end(const char *s);

/* Is the buffer of cap bytes at buf a string?
 *
 * Requires: cap bytes at buf are readable. cap may be 0, and then no byte of buf is
 *           read at all (buf is not dereferenced).
 * Reads:    at most cap bytes, and stops at the first NUL.
 * Returns:  1 if at least one of the cap bytes is a NUL, else 0. A return of 1 is the
 *           promise that every <string.h> function may be handed buf; 0 means the
 *           bytes are data, not a string, and strlen on them is undefined. */
int buf_is_terminated(const char *buf, size_t cap);

/* Fills the buffer of cap bytes at buf with copies of c, and always leaves a string.
 *
 * Requires: cap bytes at buf are writable. cap may be 0.
 * Writes:   min(n, cap - 1) copies of c at buf[0]..., then one terminator, so at most
 *           cap bytes in all and never a byte at buf[cap] or beyond. If cap == 0,
 *           writes nothing. Bytes after the terminator it writes keep their old value.
 * Returns:  the number of copies of c it wrote, which is min(n, cap - 1), or 0 when
 *           cap == 0. A return value smaller than n means the request did not fit.
 * Note:     c is a char and is stored as it is, including a byte above 0x7F and
 *           including '\0' itself. n may be any size_t, up to SIZE_MAX. */
size_t buf_set(char *buf, size_t cap, char c, size_t n);

#endif
