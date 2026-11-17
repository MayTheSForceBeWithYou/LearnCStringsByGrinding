/* countreps.h -- s01-r03-count-reps: four counts, each one pass over the string.
 *
 * In all four, s is a string, is borrowed, and is not modified. The terminator is not
 * a character of the string and is never counted. */
#ifndef COUNTREPS_H
#define COUNTREPS_H

#include <stddef.h>

/* How many bytes of s are equal to c. c == '\0' gives 0: the terminator is not part
 * of the string. A byte above 0x7F is counted like any other. */
size_t count_byte(const char *s, char c);

/* How many words s has, a word being a run of one or more bytes that are not spaces
 * (' '), tabs ('\t') or newlines ('\n'). Runs of separators, and separators at either
 * end, do not make empty words: "  a  bb  " has two. */
size_t count_words(const char *s);

/* How many lines s has. A line is the text up to a '\n' or up to the terminator, and
 * a final line without a newline counts. The empty string has no lines; "\n" has one;
 * "a\nb" has two; "a\n" has one. */
size_t count_lines(const char *s);

/* The length of the longest run of one repeated byte in s. 0 for the empty string;
 * "aabbba" is 3. */
size_t longest_run(const char *s);

#endif
