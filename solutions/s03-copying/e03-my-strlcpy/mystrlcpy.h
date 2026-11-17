/* mystrlcpy.h -- s03-e03-my-strlcpy: the truncating copy that says so. */
#ifndef MYSTRLCPY_H
#define MYSTRLCPY_H

#include <stddef.h>

/* Copies the string src into the buffer dst of `size` bytes, truncating if it must.
 *
 * Writes:   if size > 0: the first min(strlen(src), size - 1) characters of src, then
 *           one terminator. dst is always a string afterwards. No padding: the bytes
 *           after the terminator are not touched.
 *           if size == 0: nothing. dst is not read or written.
 * Reads:    all of src up to and including its terminator (it must: the return value
 *           is src's length), exactly once.
 * Requires: src is a string. dst has `size` bytes. They do not overlap.
 * Returns:  strlen(src): the length of the string it tried to create.
 *           The copy was truncated if and only if the return value is >= size. */
size_t my_strlcpy(char *restrict dst, const char *restrict src, size_t size);

#endif
