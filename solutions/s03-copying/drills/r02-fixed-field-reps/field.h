/* field.h -- s03-r02-fixed-field-reps: NUL-padded fixed-width fields.
 *
 * A field is `width` bytes. A value shorter than the field is followed by NUL bytes
 * up to the width. A value of exactly `width` characters fills the field and has NO
 * terminator. A field is therefore not a string, and none of these functions may
 * read or write field[width]. */
#ifndef FIELD_H
#define FIELD_H

#include <stddef.h>

/* Stores the string s in the field: its first `width` characters at most, NUL-padded
 * to the full width. Returns 1 if s was cut (strlen(s) > width), else 0. */
int field_put(char *field, size_t width, const char *s);

/* The number of characters in the field: 0 .. width. */
size_t field_len(const char *field, size_t width);

/* Copies the field's value out as a string into dst (cap bytes, cap >= 1), cutting it
 * to cap - 1 characters if necessary. Returns the field's length (field_len), so a
 * result >= cap means "truncated". */
size_t field_get(char *dst, size_t cap, const char *field, size_t width);

/* 1 if the field's value equals the string s, else 0. */
int field_eq(const char *field, size_t width, const char *s);

#endif
