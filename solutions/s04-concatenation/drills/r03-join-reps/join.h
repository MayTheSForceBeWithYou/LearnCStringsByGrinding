/* join.h -- s04-r03-join-reps: joining n strings with a separator into a fixed buffer.
 *
 * All three follow the snprintf contract: write what fits, always terminate when the
 * buffer has a byte to spare, and return the length of the string that was WANTED, so
 * that a return value >= cap means truncated and ret + 1 is the size that would have
 * been enough.
 *
 * Truncation here is by characters, not by whole parts: a part that only half fits is
 * written as far as it goes.
 */
#ifndef JOIN_H
#define JOIN_H

#include <stddef.h>

/* The length the join would need: the parts' lengths plus one separator between each
 * pair. No buffer, no writing. n may be 0 (length 0). */
size_t join_len(const char *const *parts, size_t n, const char *sep);

/* Joins the n parts into dst (capacity cap) with sep between them.
 * Returns join_len(parts, n, sep). cap may be 0, and then nothing is written. */
size_t join_into(char *dst, size_t cap, const char *const *parts, size_t n, const char *sep);

/* Joins only as many whole parts as fit, and reports how many that was through
 * *fitted (which may be NULL). The result is never a part cut in half.
 * Returns join_len of the parts that were written. */
size_t join_prefix(char *dst, size_t cap, const char *const *p, size_t n, const char *sep, size_t *fitted);

#endif
