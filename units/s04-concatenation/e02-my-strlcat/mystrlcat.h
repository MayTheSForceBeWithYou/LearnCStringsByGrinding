/* mystrlcat.h -- s04-e02-my-strlcat: the BSD append, written by hand. */

/* The BSD append is not in ISO C or in POSIX.1-2008, so this course cannot call it;
 * you write it. Its contract is the one worth knowing, because it is the only append in
 * common use that takes the destination's SIZE rather than a count of characters to
 * add, and reports what it would have needed. */
#ifndef MYSTRLCAT_H
#define MYSTRLCAT_H

#include <stddef.h>

/* Appends the string src to the string in dst, which has `size` bytes in all.
 *
 * Requires: src is a string. dst has `size` readable and writable bytes. `size` may be
 *           0. dst does NOT have to be terminated: if there is no terminator among its
 *           `size` bytes, this function appends nothing and reports `size` plus the
 *           length of src (see Returns).
 * Writes:   never more than `size` bytes in all, and never a byte at dst[size] or
 *           beyond. When there is room for anything at all, the result is terminated.
 *           Nothing is written when size == 0, or when dst holds no terminator.
 * Reads:    at most `size` bytes of dst, and all of src -- every time, even when
 *           nothing fits, because the return value needs src's whole length.
 * Returns:  the length of the string it tried to create: the length of dst's text
 *           plus the length of src. A return value >= size means the result was
 *           truncated, and ret + 1 is the size that would have been enough. */
size_t my_strlcat(char *dst, const char *src, size_t size);

#endif
