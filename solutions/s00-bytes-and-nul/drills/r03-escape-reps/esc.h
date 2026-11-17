/* esc.h -- s00-r03-escape-reps: showing a buffer's real contents.
 *
 * The rendering, one byte at a time:
 *
 *   0x00          \0
 *   0x0A 0x09 0x0D   \n  \t  \r
 *   0x5C ('\')    \\
 *   0x20 .. 0x7E  the character itself
 *   anything else \xHH, two upper-case hex digits
 *
 * The three rendering functions follow one contract, the same one snprintf has: they
 * write into out, which has cap bytes; they always terminate when cap > 0; they write
 * nothing at all when cap == 0; and they return the number of characters the complete
 * rendering needs, not counting the terminator. So a return value >= cap means the
 * text in out was cut short. */
#ifndef ESC_H
#define ESC_H

#include <stddef.h>

/* The hex digit for a value from 0 to 15: '0' to '9' then 'A' to 'F'.
 * Returns -1 for any other value. */
int esc_hex_digit(unsigned v);

/* Renders the single byte b. Needs at most 4 characters. */
size_t esc_one(char *out, size_t cap, unsigned char b);

/* Renders the n bytes at p, one after another, with nothing between them. */
size_t esc_buf(char *out, size_t cap, const void *p, size_t n);

/* Renders the string s and its terminator, so that the \0 at the end is visible. */
size_t esc_str(char *out, size_t cap, const char *s);

#endif
