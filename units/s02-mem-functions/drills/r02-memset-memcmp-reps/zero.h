/* zero.h -- s02-r02-memset-memcmp-reps: clearing and comparing ranges of bytes.
 *
 * Four questions a program asks about raw bytes, each one line of memset or memcmp
 * once you have the arithmetic right. Nothing here involves a terminator.
 */
#ifndef ZERO_H
#define ZERO_H

#include <stddef.h>

/* Sets every byte of the n elements of `size` bytes at base to 0.
 * n or size may be 0, and then nothing is written. */
void zero_array(void *base, size_t n, size_t size);

/* 1 if the n bytes at a equal the n bytes at b, else 0. n == 0 gives 1: two empty
 * ranges are equal. */
int ranges_equal(const void *a, const void *b, size_t n);

/* The number of leading bytes that a and b have in common, looking at no more than n
 * bytes of either. Returns 0 if the first bytes differ, and n if all n are equal. */
size_t common_prefix(const void *a, const void *b, size_t n);

/* 1 if all n bytes at p are equal to (unsigned char)c, else 0. n == 0 gives 1. */
int all_equal_to(const void *p, int c, size_t n);

#endif
