/* memops.h -- s02-e07-swap-reverse-rotate: three array operations built out of
 * memcpy and memmove, for elements of any size.
 *
 * Each function takes a base pointer, a number of elements `n` and an element size
 * `size` in bytes, the way qsort does. The elements may be anything: bytes, ints,
 * structs, pointers. Nothing here looks at an element's contents.
 *
 * None of these allocates. Where a temporary is needed, it is a small buffer on the
 * stack, used repeatedly -- so an element may be larger than that buffer, and the
 * code has to cope by moving an element in pieces.
 */
#ifndef MEMOPS_H
#define MEMOPS_H

#include <stddef.h>

/* The scratch buffer the three functions may use. An element may be larger. */
#define MEMOPS_CHUNK 32

/* Exchanges the `size` bytes at a with the `size` bytes at b.
 *
 * Requires: `size` bytes are readable and writable at each. The two regions do not
 *           overlap, unless a == b, which is allowed and does nothing.
 * Writes:   exactly `size` bytes at each. */
void mem_swap(void *a, void *b, size_t size);

/* Reverses the order of the n elements of `size` bytes each at base.
 *
 * Requires: n * size bytes are readable and writable at base.
 * Writes:   every byte of the array, unless n is 0 or 1, in which case nothing. */
void mem_reverse(void *base, size_t n, size_t size);

/* Rotates the n elements at base left by k places: the element at index k moves to
 * index 0, and the first k elements move to the end, keeping their order.
 *
 * Requires: n * size bytes are readable and writable at base. k may be any value,
 *           including 0 and values of n or more; rotating by k is the same as
 *           rotating by k % n (and rotating an empty array does nothing).
 * Writes:   the array, in place. */
void mem_rotate_left(void *base, size_t n, size_t size, size_t k);

#endif
