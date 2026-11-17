/* stats.h -- s01-e03-empty-wraps: counting characters of one class in a string. */
#ifndef STATS_H
#define STATS_H

#include <stddef.h>

/* How many of the characters of s are ASCII upper-case letters, 'A' to 'Z'.
 *
 * s is borrowed and is not modified. s is a string; every byte up to and including
 * its terminator is read, and no byte after it.
 * Returns the count; 0 for the empty string. */
size_t count_upper(const char *s);

/* How many of the characters of s are ASCII digits, '0' to '9'. Same contract. */
size_t count_digits(const char *s);

#endif
