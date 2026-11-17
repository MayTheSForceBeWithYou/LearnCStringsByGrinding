/* text.h -- s02-e03-overlap-report: editing a text buffer in place. */
#ifndef TEXT_H
#define TEXT_H

#include <stddef.h>

/* How many bytes a deletion has to move: everything after the deleted range, plus the
 * terminator. */
size_t text_move_count(size_t len, size_t at, size_t n);

/* Deletes n bytes at offset `at` from the string in buf, which has length len.
 *
 * buf holds a string of len characters and a terminator. at + n must be at most len.
 * The bytes after the deleted range, and the terminator, move down by n.
 * Returns the new length. */
size_t text_delete(char *buf, size_t len, size_t at, size_t n);

#endif
