/* label.h -- s04-e04-strncat-size: building a short label in a fixed buffer. */
#ifndef LABEL_H
#define LABEL_H

#include <stddef.h>

/* Appends `src` to the string in dst, which has `cap` bytes in all, truncating if it
 * does not fit.
 *
 * dst is a string and is still one afterwards. Returns 0 if the whole of src was
 * appended, or -1 if it was truncated (and then dst holds as much as fitted).
 * Never writes dst[cap] or beyond. */
int label_append(char *dst, size_t cap, const char *src);

/* Builds "<kind>: <text>" in dst, truncating if it does not fit. Returns 0, or -1 if
 * anything was truncated. */
int label_build(char *dst, size_t cap, const char *kind, const char *text);

#endif
