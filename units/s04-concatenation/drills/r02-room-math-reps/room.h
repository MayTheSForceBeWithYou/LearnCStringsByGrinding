/* room.h -- s04-r02-room-math-reps: the arithmetic of "does it fit?".
 *
 * Four questions that come before every append, and four chances to get an unsigned
 * subtraction or an unsigned sum wrong. `cap` is always a buffer's total size in
 * bytes, terminator included; `len` is a count of characters.
 *
 * Every function here must be correct for the extremes: cap == 0, len == cap - 1,
 * len == cap, and lengths near SIZE_MAX. No expression may wrap.
 */
#ifndef ROOM_H
#define ROOM_H

#include <stddef.h>

/* How many more characters fit in a buffer of `cap` bytes that already holds `len`
 * of them. 0 when cap is 0, and 0 when len is cap - 1 or more (including the case
 * where len is somehow larger than the buffer). */
size_t room_left(size_t cap, size_t len);

/* 1 if appending `add` more characters to a buffer of `cap` bytes holding `len` of
 * them fits, else 0. Fitting means len + add + 1 <= cap, and the test must give the
 * right answer even when len + add would wrap. */
int will_fit(size_t cap, size_t len, size_t add);

/* The buffer size needed to hold the two-part string of `a` characters and `b`
 * characters: a + b + 1. Returns 0 if that sum would wrap, which is the only way this
 * function reports a problem. */
size_t size_needed(size_t a, size_t b);

/* The same for three parts and two separators of one character each:
 * a + 1 + b + 1 + c + 1. Returns 0 if any part of that sum would wrap. */
size_t size_needed3(size_t a, size_t b, size_t c);

#endif
