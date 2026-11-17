/* scratch.h -- s02-r03-mem-from-scratch: all five mem functions, from a blank file.
 *
 * The prototypes are the library's, with s_ in front. Each contract is the C standard's,
 * including the parts that are easy to forget: the value argument is converted to
 * unsigned char, bytes compare as unsigned char, only the sign of the comparison is
 * fixed, n may be 0 everywhere, and only s_memmove is defined for overlap.
 */
#ifndef SCRATCH_H
#define SCRATCH_H

#include <stddef.h>

/* n bytes from src to dst; the regions must not overlap. Returns dst. */
void *s_memcpy(void *restrict dst, const void *restrict src, size_t n);

/* n bytes from src to dst; the regions may overlap. Returns dst. */
void *s_memmove(void *dst, const void *src, size_t n);

/* n copies of (unsigned char)c at s. Returns s. */
void *s_memset(void *s, int c, size_t n);

/* Compares n bytes as unsigned char. Negative, zero or positive; only the sign is
 * fixed. n == 0 gives 0. */
int s_memcmp(const void *a, const void *b, size_t n);

/* The first byte equal to (unsigned char)c among the first n at s, or NULL. */
void *s_memchr(const void *s, int c, size_t n);

#endif
