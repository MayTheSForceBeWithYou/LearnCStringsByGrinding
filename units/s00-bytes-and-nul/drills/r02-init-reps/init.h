/* init.h -- s00-r02-init-reps: putting bytes into a buffer by hand, and leaving a
 * string behind every time.
 *
 * In all four, cap is the total size of buf in bytes, terminator included, and may be
 * 0. No function here writes buf[cap] or any byte beyond it, and none of them reads or
 * writes a byte of buf it did not have to. */
#ifndef INIT_H
#define INIT_H

#include <stddef.h>

/* Makes buf the empty string. Writes exactly one byte, buf[0] = '\0'.
 * Returns 0, or -1 without writing anything if cap == 0. */
int init_empty(char *buf, size_t cap);

/* Writes min(n, cap - 1) copies of c, then a terminator.
 * Returns the number of copies written; 0 if cap == 0 (and then nothing is written).
 * n may be as large as SIZE_MAX. */
size_t init_repeat(char *buf, size_t cap, char c, size_t n);

/* Copies the string lit into buf byte by byte, stopping at cap - 1 characters, and
 * always terminates. lit is borrowed and is read once, up to its terminator or up to
 * the byte that does not fit, whichever comes first.
 * Returns the number of characters copied; 0 if cap == 0 (nothing written). */
size_t init_literal(char *buf, size_t cap, const char *lit);

/* 1 if one of the cap bytes at buf is a NUL, else 0. Reads at most cap bytes and
 * stops at the first NUL; reads nothing at all when cap == 0. */
int init_terminated(const char *buf, size_t cap);

#endif
