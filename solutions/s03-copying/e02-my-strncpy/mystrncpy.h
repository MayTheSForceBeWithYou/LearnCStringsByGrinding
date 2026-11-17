/* mystrncpy.h -- s03-e02-my-strncpy: the fixed-width field copies. */
#ifndef MYSTRNCPY_H
#define MYSTRNCPY_H

#include <stddef.h>

/* Fills the n bytes at dst from the string src.
 *
 * Writes:   exactly n bytes, always. First the characters of src, stopping after n
 *           characters or at src's terminator, whichever comes first; then NUL bytes
 *           until n bytes have been written.
 *           If src has n or more characters, no NUL is written at all: dst is then
 *           NOT a string.
 * Reads:    at most n bytes of src, and nothing after src's terminator.
 * Requires: dst has room for n bytes. src is a string, or has at least n readable
 *           bytes. They do not overlap. n may be 0 (nothing is read or written).
 * Returns:  dst. */
char *my_strncpy(char *restrict dst, const char *restrict src, size_t n);

/* The same n bytes, with a more useful return value.
 *
 * Returns:  a pointer to the first NUL it wrote (dst + strlen(src)) when src is
 *           shorter than n; dst + n when no NUL was written. */
char *my_stpncpy(char *restrict dst, const char *restrict src, size_t n);

#endif
