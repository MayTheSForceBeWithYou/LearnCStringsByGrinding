/* guard.h -- destination buffers of an exact capacity, for testing functions that write
 * into a caller's buffer.
 *
 *     char *dst = guard_new(8);            8 usable bytes, all '#'
 *     my_strncpy(dst, "hello", 8);
 *     check_bytes(dst, "hello\0\0\0", 8, "padded to n");
 *     check_guard(dst, "nothing written outside the 8 bytes");
 *     guard_free(dst);
 *
 * Plain build: the 8 bytes sit between two 32-byte fences of 0xA5; guard_ok() says
 * whether the fences are intact. A write of one byte too many is caught by the check.
 *
 * ASan build: the block is exactly 8 bytes of heap, so the same write is reported by
 * ASan at the byte, with a stack trace ("0 bytes after 8-byte region").
 *
 * A capacity of 0 is allowed: no byte of the returned pointer may be touched.
 * These blocks are not counted by dsalloc (they belong to the harness, not to you).
 */
#ifndef COURSE_GUARD_H
#define COURSE_GUARD_H

#include <stddef.h>

#define GUARD_FILL '#'

/* A new buffer of cap usable bytes, each set to GUARD_FILL. Never returns NULL
 * (the program exits with a message if the harness itself runs out of memory). */
char *guard_new(size_t cap);

/* A new buffer holding exactly the n bytes at src: for sources that must not be read
 * past their end (an unterminated field, a string and its terminator and nothing more). */
char *guard_copy(const void *src, size_t n);

/* Sets every usable byte back to GUARD_FILL. */
void guard_reset(char *p);

/* 1 if nothing outside the usable bytes has been written, else 0 (plain build).
 * Always 1 in the ASan build, where ASan stops the program at the bad write instead. */
int guard_ok(const char *p);

/* The capacity p was created with. */
size_t guard_cap(const char *p);

void guard_free(char *p);

#endif
