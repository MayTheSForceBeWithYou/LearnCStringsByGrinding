/* mycopy.h -- s03-e01-my-strcpy-stpcpy: the two unbounded string copies. */
#ifndef MYCOPY_H
#define MYCOPY_H

/* Copies the string src, including its terminator, to dst.
 *
 * Requires: src is a string. dst has room for strlen(src) + 1 bytes (the caller
 *           guarantees it; this function has no way to check). dst and src do not
 *           overlap.
 * Writes:   exactly strlen(src) + 1 bytes, starting at dst[0]. No byte after the
 *           terminator is touched.
 * Returns:  dst. */
char *my_strcpy(char *restrict dst, const char *restrict src);

/* The same copy, with a more useful return value.
 *
 * Requires, writes: exactly as my_strcpy.
 * Returns:  a pointer to the terminator it wrote, that is, dst + strlen(src). */
char *my_stpcpy(char *restrict dst, const char *restrict src);

#endif
