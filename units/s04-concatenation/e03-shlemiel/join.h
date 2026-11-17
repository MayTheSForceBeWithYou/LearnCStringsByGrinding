/* join.h -- s04-e03-shlemiel: joining many words into one buffer. */
#ifndef JOIN_H
#define JOIN_H

#include <stddef.h>

/* Joins n words into dst, separated by one space, with no space at either end.
 *
 * dst is the caller's buffer of `cap` bytes and is always terminated when cap > 0.
 * words is an array of n strings, borrowed and not modified.
 *
 * Truncation is by whole pieces: a word (or a separator) is appended only if it fits
 * entirely, and once something has not fitted, nothing after it fits either. So with
 * cap 8 and the words alpha, beta, gamma the buffer holds "alpha " -- the separator
 * fitted, the next word did not.
 *
 * Returns the length of the string it tried to create, like snprintf: a return value
 * >= cap means the result was truncated, and ret + 1 is the capacity that would have
 * been enough. */
size_t join_words(char *dst, size_t cap, const char *const *words, size_t n);

#endif
