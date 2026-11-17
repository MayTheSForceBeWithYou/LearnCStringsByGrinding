/* name.h -- s01-e06-narrow-length: a record whose wire format stores the name's
 * length in one byte.
 *
 * The format is fixed and is not the defect: a name is at most 255 characters, and the
 * length byte is how a reader of the record knows where the name ends. The record's
 * own buffer is larger than that so that one buffer can be reused for every record.
 */
#ifndef NAME_H
#define NAME_H

#include <stddef.h>

#define NAME_CAP 512 /* bytes in the record's buffer */
#define NAME_MAX 255 /* the longest name the one-byte length field can describe */

typedef struct {
    char buf[NAME_CAP];
    unsigned char len; /* the length of the name in buf, as the wire format stores it */
} Name;

/* Sets the record's name to the string s.
 *
 * `s` is borrowed and copied into the record. Returns 0 on success, with `len` holding
 * the name's length and `buf` holding the name and a terminator.
 * Returns -1 and changes nothing at all if the name cannot be stored: that is, if it
 * is longer than NAME_MAX characters. */
int name_set(Name *n, const char *s);

/* The length of the record's name, from the record's own length field. */
size_t name_len(const Name *n);

/* Copies the record's name into dst (capacity cap) as a string, using the record's
 * length field rather than searching for a terminator. Returns the name's length,
 * like snprintf: a return value >= cap means the copy was truncated. */
size_t name_get(const Name *n, char *dst, size_t cap);

#endif
