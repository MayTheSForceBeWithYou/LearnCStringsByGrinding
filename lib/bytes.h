/* bytes.h -- the course's byte notation: what a buffer really contains, made visible.
 *
 *     h e l l o \0 # #        is written        hello\0##
 *
 * Printable ASCII is shown as itself; the terminator as \0; newline, tab and carriage
 * return as \n \t \r; a backslash as \\; every other byte as \xHH (two upper-case hex
 * digits). Lessons, READMEs, answers.txt and harness failure messages all use it.
 */
#ifndef COURSE_BYTES_H
#define COURSE_BYTES_H

#include <stddef.h>

/* Renders the n bytes at p into out (capacity cap, always NUL-terminated when cap > 0).
 * Returns the number of characters the full rendering needs, like snprintf: a return
 * value >= cap means the text in out was cut short. */
size_t bytes_notation(const void *p, size_t n, char *out, size_t cap);

/* The same, into one of four rotating static buffers (enough for one printf call).
 * Output longer than 1023 characters is cut and ends in "...". */
const char *bytes_show(const void *p, size_t n);

#endif
