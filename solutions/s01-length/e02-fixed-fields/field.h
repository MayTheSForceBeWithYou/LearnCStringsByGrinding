/* field.h -- s01-e02-fixed-fields: reading NUL-padded fixed-width record fields.
 *
 * A field is `width` bytes holding a character sequence, padded with NUL bytes if the
 * text is shorter than the field. A field whose text is exactly `width` characters
 * long has no padding and no terminator: it is not a string, and no function that
 * looks for a terminator may be given it.
 *
 * In all three, `field` is borrowed, is never modified, and has exactly `width`
 * readable bytes. Reading field[width] is a defect, not a convenience. */
#ifndef FIELD_H
#define FIELD_H

#include <stddef.h>

/* The length of the text in the field: the number of bytes before the first NUL, or
 * `width` if there is no NUL among the `width` bytes.
 *
 * Reads:   at most `width` bytes, stopping at the first NUL.
 * Returns: 0 .. width. A return value equal to `width` means the field is full and
 *          holds no terminator. width == 0 gives 0 and reads nothing. */
size_t field_len(const char *field, size_t width);

/* Do two fields of the same width hold the same text?
 *
 * The padding is not part of the text, so two fields are equal when their lengths are
 * equal and their text bytes are equal. Bytes after the first NUL of either field are
 * not compared.
 * Reads:   at most `width` bytes of each.
 * Returns: 1 if the text is equal, else 0. Two empty fields are equal; width == 0
 *          gives 1 and reads nothing. */
int field_eq(const char *a, const char *b, size_t width);

/* Copies the field's text into dst as a string.
 *
 * dst is the caller's buffer of `cap` bytes; it may be 0 bytes, and then nothing is
 * written. At most cap - 1 characters are copied, and dst is always terminated when
 * cap > 0.
 * Returns: the length of the field's text, that is, field_len(field, width) -- the
 *          length it tried to produce. So a return value >= cap means the text was
 *          truncated, and ret + 1 is the capacity that would have been enough. */
size_t field_to_cstr(char *dst, size_t cap, const char *field, size_t width);

#endif
