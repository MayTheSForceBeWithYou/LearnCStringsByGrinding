/* walk.h -- s00-r01-walk-reps: five questions about a string, each answered by one
 * walk from its first byte to its terminator.
 *
 * In all five, s is a string: the bytes from s[0] to the first NUL are readable, and
 * no byte after that terminator may be read. */
#ifndef WALK_H
#define WALK_H

#include <stddef.h>

/* The number of bytes before the terminator. 0 for the empty string. */
size_t walk_len(const char *s);

/* A pointer to the terminator itself, which is s for the empty string. Never NULL. */
const char *walk_end(const char *s);

/* How many of the bytes before the terminator are equal to c. Counts a byte above
 * 0x7F correctly; c == '\0' is 0, because the terminator is not part of the string. */
size_t walk_count(const char *s, char c);

/* The last character of s, as a value from 0 to 255, or -1 if s is empty. */
int walk_last(const char *s);

/* 1 if s is the empty string, else 0. Reads one byte. */
int walk_is_empty(const char *s);

#endif
