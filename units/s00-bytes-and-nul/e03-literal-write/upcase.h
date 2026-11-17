/* upcase.h -- s00-e03-literal-write: upper-casing in place. This module is correct. */
#ifndef UPCASE_H
#define UPCASE_H

/* Upper-cases the ASCII lower-case letters of the string s, in place.
 *
 * Requires: s is a string, and its bytes are in storage that may be written. Every
 *           other byte of s (digits, punctuation, spaces, bytes above 0x7F) is left
 *           exactly as it is.
 * Writes:   the bytes of s, from s[0] to the byte before its terminator. The
 *           terminator is not touched and nothing after it is.
 * Returns:  nothing. There is no error case: the function cannot tell writable
 *           storage from read-only storage, and does not try. */
void upcase(char *s);

#endif
