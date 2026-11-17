/* mymem.h -- s02-e02-my-memset-memcmp-memchr: the other three mem functions.
 *
 * All three take a byte count and look at nothing else: no terminator, no character
 * class. n may be 0 in all three, and then no byte is read or written.
 *
 * Two of them take the byte as an `int`, for the same historical reason printf does:
 * before prototypes, every char argument was promoted to int anyway. In both cases the
 * value is converted to unsigned char before use, so 'A', 65 and 0x141 all mean the
 * same byte.
 */
#ifndef MYMEM_H
#define MYMEM_H

#include <stddef.h>

/* Writes n copies of (unsigned char)c to the n bytes at s.
 *
 * Requires: n bytes are writable at s.
 * Writes:   exactly n bytes. Reads none.
 * Returns:  s. */
void *my_memset(void *s, int c, size_t n);

/* Compares the first n bytes at a and b as unsigned char.
 *
 * Requires: n bytes are readable at a and at b.
 * Reads:    at most n bytes of each; it may stop at the first difference.
 * Returns:  a negative value if the first differing byte is smaller in a, a positive
 *           value if it is larger, and 0 if the n bytes are equal. Only the SIGN is
 *           specified: never compare the result with 1 or -1. n == 0 gives 0. */
int my_memcmp(const void *a, const void *b, size_t n);

/* Finds the first byte equal to (unsigned char)c among the first n bytes at s.
 *
 * Requires: n bytes are readable at s.
 * Reads:    at most n bytes, and stops at the byte it finds.
 * Returns:  a pointer to that byte, or NULL if it is not among the n. n == 0 gives
 *           NULL and reads nothing. A NUL byte is found like any other. */
void *my_memchr(const void *s, int c, size_t n);

#endif
