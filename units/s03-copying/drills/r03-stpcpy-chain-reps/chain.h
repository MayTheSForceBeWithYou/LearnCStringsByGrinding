/* chain.h -- s03-r03-stpcpy-chain-reps: build strings with an end pointer.
 *
 * The rule of the drill: every function keeps a pointer to where the next character
 * goes and advances it as it writes. Nothing is measured after it has been written:
 * no strlen on the destination, no strcat. */
#ifndef CHAIN_H
#define CHAIN_H

#include <stddef.h>

/* Writes a then b into dst. The caller guarantees room. Returns a pointer to the
 * terminator of the result. */
char *chain_two(char *dst, const char *a, const char *b);

/* Writes "a/b/c" into dst (the three parts separated by single slashes; the parts
 * are copied as they are). The caller guarantees room. Returns the length of the
 * result. */
size_t chain_path(char *dst, const char *a, const char *b, const char *c);

/* Writes n copies of s into dst. n == 0 gives the empty string. The caller
 * guarantees room. Returns a pointer to the terminator of the result. */
char *chain_repeat(char *dst, const char *s, size_t n);

/* Writes the `count` strings of parts[], one after another, into dst (cap bytes).
 * If everything fits, returns a pointer to the terminator. If it does not, returns
 * NULL and, when cap > 0, leaves dst as the empty string. Checks the room BEFORE
 * writing each part. */
char *chain_bounded(char *dst, size_t cap, const char *const *parts, size_t count);

#endif
