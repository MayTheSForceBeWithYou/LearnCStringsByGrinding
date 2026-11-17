/* mylen.h -- s01-e01-my-strlen: the two measuring functions of the library. */
#ifndef MYLEN_H
#define MYLEN_H

#include <stddef.h>

/* The length of the string s: the number of bytes before its terminator.
 *
 * Requires: s is a string. Every byte from s[0] up to and including the first NUL is
 *           readable; a buffer with no NUL in it is not a valid argument, and this
 *           function cannot detect one.
 * Reads:    exactly my_strlen(s) + 1 bytes, and no byte after the terminator.
 * Returns:  the number of bytes before the terminator; 0 for the empty string. There
 *           is no error return. */
size_t my_strlen(const char *s);

/* The length of the character sequence at s, measured over at most maxlen bytes.
 *
 * Requires: the first min(maxlen, <position of the first NUL> + 1) bytes at s are
 *           readable. s need NOT be terminated: that is the point of this function.
 *           maxlen may be 0, and then no byte of s is read at all.
 * Reads:    at most maxlen bytes, and stops at the first NUL.
 * Returns:  the number of bytes before the first NUL, or maxlen if there is no NUL
 *           among the first maxlen bytes. So a return value equal to maxlen means
 *           "no terminator was found", and a smaller one is a true length. */
size_t my_strnlen(const char *s, size_t maxlen);

#endif
