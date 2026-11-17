/* endreps.h -- s01-r02-end-pointer-reps: measuring with two pointers instead of a
 * length. This is how an appender keeps its place: one pointer at the end of what has
 * been written, one at the end of the buffer, and the arithmetic between them. */
#ifndef ENDREPS_H
#define ENDREPS_H

#include <stddef.h>

/* A pointer to the terminator of the string s: &s[strlen(s)], never NULL. For the
 * empty string it is s itself. */
char *end_of(char *s);

/* A pointer to the last character of s, or NULL if s is empty. */
char *last_char(char *s);

/* The number of bytes from begin to end, where both point into one object and begin
 * is not after end. */
size_t len_between(const char *begin, const char *end);

/* How many more characters can be written at p, given that the buffer ends at
 * `buf_end` (which points one past its last byte) and the result must stay a string.
 * Returns 0 when there is room for nothing but the terminator, and 0 when p is
 * already at buf_end. */
size_t room_left(const char *p, const char *buf_end);

#endif
