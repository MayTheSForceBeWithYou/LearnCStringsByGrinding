/* bcopy.h -- s03-r01-bounded-copy-reps: one bounded copy, four ways.
 * In all four, cap is the total size of dst, terminator included, and cap >= 1. */
#ifndef BCOPY_H
#define BCOPY_H

#include <stddef.h>

/* By hand, one loop, no library call: copy at most cap - 1 characters, terminate,
 * and return strlen(src) (so a result >= cap means "truncated"). */
size_t bc_loop(char *dst, size_t cap, const char *src);

/* With snprintf and "%s". Returns 1 if the copy was truncated, else 0. */
int bc_format(char *dst, size_t cap, const char *src);

/* With strlen, then one memcpy, then the terminator. Returns 1 if truncated, else 0. */
int bc_measured(char *dst, size_t cap, const char *src);

/* All or nothing: 0 and a full copy if src fits; -1 and dst == "" if it does not. */
int bc_strict(char *dst, size_t cap, const char *src);

#endif
