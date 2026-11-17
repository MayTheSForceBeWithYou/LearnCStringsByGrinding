/* prefix.h -- s00-e03-literal-write: message prefixes such as "WARNING: ". */
#ifndef PREFIX_H
#define PREFIX_H

#include <stddef.h>

#define PREFIX_INFO 0
#define PREFIX_WARN 1
#define PREFIX_ERROR 2

/* The name of a message level, in lower case: "info", "warning" or "error".
 *
 * Returns a pointer to storage that belongs to this module and lives for the whole
 * run. The caller may read it and must not write it or free it. An unknown level
 * gives "error". */
const char *prefix_name(int level);

/* Writes the prefix for a message level into out: the level's name in capital
 * letters, then a colon and a space, then a terminator. "WARNING: " for PREFIX_WARN.
 *
 * out is the caller's buffer of cap bytes. Returns 0 on success, or -1 if the prefix
 * does not fit in cap bytes, in which case out is not written at all. */
int prefix_write(char *out, size_t cap, int level);

#endif
