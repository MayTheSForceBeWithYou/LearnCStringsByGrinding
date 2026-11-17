/* mycat.h -- s04-e01-my-strcat-strncat: the two appending functions.
 *
 * Both start by finding the terminator of dst and write from there. Neither takes the
 * size of dst: the caller is responsible for the room, and these functions cannot
 * check it.
 */
#ifndef MYCAT_H
#define MYCAT_H

#include <stddef.h>

/* Appends the string src to the string dst.
 *
 * Requires: dst is a string. src is a string. dst has room for
 *           strlen(dst) + strlen(src) + 1 bytes. The two do not overlap.
 * Writes:   strlen(src) + 1 bytes, starting at dst's old terminator.
 * Reads:    all of dst (to find its end) and all of src.
 * Returns:  dst. */
char *my_strcat(char *restrict dst, const char *restrict src);

/* Appends at most n characters of src to the string dst, then always a terminator.
 *
 * Requires: dst is a string. src is a string, or has at least n readable bytes. dst has
 *           room for strlen(dst) + n + 1 bytes -- note the + 1: this function writes up
 *           to n + 1 bytes, which is the usual mistake about it. No overlap.
 * Writes:   min(n, <characters of src>) characters, then one terminator. So at most
 *           n + 1 bytes, and always at least 1.
 * Reads:    all of dst, and at most n + 1 bytes of src (it stops at src's terminator
 *           and never reads past n).
 * Returns:  dst. There is no report of truncation. */
char *my_strncat(char *restrict dst, const char *restrict src, size_t n);

#endif
