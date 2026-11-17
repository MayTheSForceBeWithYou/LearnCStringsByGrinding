/* mymove.h -- s02-e01-my-memcpy-memmove: the two byte-moving functions.
 *
 * Neither of these looks at the bytes it moves: there is no terminator, no character
 * class, no end condition other than the count. n may be 0, and then nothing at all is
 * read or written -- but the pointers must still be valid ones.
 */
#ifndef MYMOVE_H
#define MYMOVE_H

#include <stddef.h>

/* Copies n bytes from src to dst.
 *
 * Requires: n bytes are readable at src and writable at dst, and the two regions do
 *           NOT overlap. The `restrict` qualifiers are that promise, made by the
 *           caller; this function does not check it and cannot.
 * Writes:   exactly n bytes at dst. Reads exactly n bytes at src.
 * Returns:  dst. */
void *my_memcpy(void *restrict dst, const void *restrict src, size_t n);

/* Copies n bytes from src to dst, correctly even when the two regions overlap.
 *
 * Requires: n bytes are readable at src and writable at dst. The regions MAY overlap
 *           in any way, including dst == src.
 * Writes:   exactly n bytes at dst, as if the n bytes at src had first been copied to
 *           a temporary buffer and then from there to dst (C17 7.24.2.2). No byte of
 *           the source is read after the byte that would overwrite it has been
 *           written.
 * Returns:  dst. */
void *my_memmove(void *dst, const void *src, size_t n);

#endif
