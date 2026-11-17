/* lenreps.h -- s01-r01-strlen-reps: strlen three ways, and the question it is for. */
#ifndef LENREPS_H
#define LENREPS_H

#include <stddef.h>

/* The length of the string s, written with a subscript and a counter.
 * Requires s to be a string; reads up to and including its terminator. */
size_t len_index(const char *s);

/* The same answer, written as the difference between a pointer that walked to the
 * terminator and the start. No counter. */
size_t len_diff(const char *s);

/* The length of the character sequence at s over at most maxlen bytes: the number of
 * bytes before the first NUL, or maxlen if there is none among the first maxlen.
 * s need not be terminated. maxlen may be 0, and then no byte is read. */
size_t len_bounded(const char *s, size_t maxlen);

/* Does the string s fit in a buffer of cap bytes, terminator included?
 * Returns 1 if it fits, else 0. cap may be 0 (nothing fits, not even ""). */
int len_fits(const char *s, size_t cap);

#endif
