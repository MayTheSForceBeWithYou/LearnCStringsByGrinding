/* dsalloc.h -- portable allocation accounting and fault injection.
 *
 * common.mk force-includes this header into every course translation unit
 * (`-include lib/dsalloc.h`), in every build mode. Student code calls plain
 * malloc/calloc/realloc/strdup/strndup/free; the macros below route those calls
 * through lib/dsalloc.c, which forwards to the real allocator (ASan's, in the
 * sanitizer build) and keeps a table of live blocks.
 *
 * Why not --wrap or LeakSanitizer?  `-Wl,--wrap` is a GNU ld feature that Apple ld
 * does not have, and LeakSanitizer is not supported by Apple clang on arm64. This
 * shim is plain C11 + POSIX, so leak checking and fault injection behave the same
 * on Arch Linux (gcc, clang) and macOS (Apple clang). On Linux, LeakSanitizer also
 * runs in the ASan build as a second opinion (it sees allocations made inside libc).
 *
 * Environment (read once, at the first allocation or at startup):
 *   FAIL_AT=N        the Nth allocation call fails (returns NULL, errno = ENOMEM).
 *                    Only that call. A failed realloc leaves the old block alive.
 *   FA_COUNT=1       print "dsalloc: N allocation calls" to stderr at exit.
 *   DSALLOC_LEAKS=0  do not check for leaks at exit (lesson demos only).
 *
 * At exit, if any block allocated through these macros is still live, the shim
 * prints one line per allocation site and ends the process with status 4
 * (EXIT_LEAK). Every live block counts, reachable or not: free everything.
 *
 * Harness-only API (student code must not call these):
 *   ds_live_blocks(), ds_live_bytes()  live blocks / bytes right now
 *   ds_alloc_calls()                   allocation calls so far
 *   ds_fail_nth(n)                     the nth allocation call from now fails (0 cancels)
 *
 * Rules this imposes (docs/AUTHORING.md, "Portability"):
 *   - Do not take the address of malloc/calloc/realloc/strdup/strndup. Passing
 *     `free` as a function pointer is fine (it is an object-like macro).
 *   - realloc(p, 0) is rejected with a diagnostic and abort(): its meaning differs
 *     between C17 implementations and is undefined in C23.
 *   - Feature macros: this header defines _POSIX_C_SOURCE 200809L before any
 *     system header, so strdup, strndup, getline and clock_gettime are declared.
 *     Do not define _GNU_SOURCE, _DEFAULT_SOURCE or _DARWIN_C_SOURCE.
 */
#ifndef DSALLOC_H
#define DSALLOC_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *ds_malloc_at(size_t n, const char *file, int line);
void *ds_calloc_at(size_t n, size_t size, const char *file, int line);
void *ds_realloc_at(void *p, size_t n, const char *file, int line);
char *ds_strdup_at(const char *s, const char *file, int line);
char *ds_strndup_at(const char *s, size_t n, const char *file, int line);
void ds_free(void *p);

size_t ds_live_blocks(void);
size_t ds_live_bytes(void);
unsigned long ds_alloc_calls(void);
void ds_fail_nth(unsigned long n);

#ifndef DSALLOC_NO_MACROS
#define malloc(n) ds_malloc_at((n), __FILE__, __LINE__)
#define calloc(n, size) ds_calloc_at((n), (size), __FILE__, __LINE__)
#define realloc(p, n) ds_realloc_at((p), (n), __FILE__, __LINE__)
#define strdup(s) ds_strdup_at((s), __FILE__, __LINE__)
#define strndup(s, n) ds_strndup_at((s), (n), __FILE__, __LINE__)
#define free ds_free
#endif

#endif
