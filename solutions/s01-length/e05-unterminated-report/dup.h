/* dup.h -- s01-e05-unterminated-report: heap copies of fixed-width record fields. */
#ifndef DUP_H
#define DUP_H

#include <stddef.h>

/* A heap copy of the text in a fixed-width field, as a string.
 *
 * `field` is borrowed and has exactly `width` readable bytes; it is NUL-padded if its
 * text is shorter than the field, and has no terminator at all if the text fills it.
 * Returns one owned string, which the caller frees, or NULL if allocation fails.
 * The result is always a string: field_dup("A123", 4) returns "A123". */
char *field_dup(const char *field, size_t width);

/* The number of bytes field_dup would allocate for this field. */
size_t field_dup_size(const char *field, size_t width);

#endif
