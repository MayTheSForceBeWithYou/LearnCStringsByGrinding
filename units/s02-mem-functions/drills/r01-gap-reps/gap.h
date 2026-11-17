/* gap.h -- s02-r01-gap-reps: moving bytes around inside one buffer.
 *
 * All four work on a buffer of `cap` bytes holding `len` used bytes at the front.
 * They move bytes within that one buffer, so the source and destination overlap and
 * the copy has to be the one that is defined for overlap.
 *
 * None of these looks at the bytes: there is no terminator here, only counts. Each
 * returns the new used length, or the old one unchanged if the request does not fit.
 */
#ifndef GAP_H
#define GAP_H

#include <stddef.h>

/* Opens a gap of n bytes at offset `at`: the bytes from `at` to `len` move up by n.
 * The gap's own bytes are not written, so they keep whatever was there.
 * Does nothing and returns len if at > len, or if len + n > cap.
 * Returns the new used length. */
size_t gap_open(char *buf, size_t cap, size_t len, size_t at, size_t n);

/* Closes a gap of n bytes at offset `at`: the bytes from `at + n` to `len` move down
 * by n, and the n bytes at the end are no longer used (they are not cleared).
 * Does nothing and returns len if at + n > len.
 * Returns the new used length. */
size_t gap_close(char *buf, size_t cap, size_t len, size_t at, size_t n);

/* Deletes the range [from, to) from the used bytes. from <= to <= len is required;
 * anything else does nothing. Returns the new used length. */
size_t gap_delete(char *buf, size_t cap, size_t len, size_t from, size_t to);

/* Inserts the n bytes at src at offset `at`, moving what was there up.
 * src does not overlap buf. Does nothing and returns len if at > len or
 * len + n > cap. Returns the new used length. */
size_t gap_insert(char *buf, size_t cap, size_t len, size_t at, const char *src, size_t n);

#endif
