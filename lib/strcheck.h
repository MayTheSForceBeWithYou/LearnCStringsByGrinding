/* strcheck.h -- string-aware checks on top of course.h.
 *
 *     check_str(buf, "hello", "copies the whole word");
 *     check_bytes(dst, "hel#####", 8, "strncpy with n == 3 writes three bytes and stops");
 *     check_guard(dst, "nothing written outside the buffer");
 *
 * On failure each prints the usual FAIL line, then what it got and what it wanted in
 * byte notation (lib/bytes.h), so an unwritten byte (#), a missing terminator or a
 * stray \0 is visible. Like check(), none of them aborts.
 *
 * Everything here is static inline on purpose: course.h keeps its failure counter per
 * translation unit, so checks must live in the file that calls checks_done().
 */
#ifndef COURSE_STRCHECK_H
#define COURSE_STRCHECK_H

#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "course.h"
#include "guard.h"

/* -1, 0 or 1: the only thing the standard promises about a comparison function. */
static inline int sign_of(int v)
{
    return (v > 0) - (v < 0);
}

static inline void course_check_str_(const char *got, const char *want, const char *msg,
                                     const char *file, int line)
{
    if (got != NULL && want != NULL && strcmp(got, want) == 0)
        return;
    course_check_(0, msg, file, line);
    printf("       got:  %s\n", got ? bytes_show(got, strlen(got) + 1) : "(null pointer)");
    printf("       want: %s\n", want ? bytes_show(want, strlen(want) + 1) : "(null pointer)");
}

static inline void course_check_bytes_(const void *got, const void *want, size_t n, const char *msg,
                                       const char *file, int line)
{
    if (memcmp(got, want, n) == 0)
        return;
    const unsigned char *g = got, *w = want;
    size_t i = 0;
    while (g[i] == w[i])
        i++;
    course_check_(0, msg, file, line);
    printf("       got:  %s\n", bytes_show(got, n));
    printf("       want: %s\n", bytes_show(want, n));
    printf("       first difference at index %zu of %zu\n", i, n);
}

static inline void course_check_guard_(const char *p, const char *msg, const char *file, int line)
{
    if (!guard_ok(p))
        course_check_(0, msg, file, line);
}

#define check_str(got, want, msg) course_check_str_((got), (want), (msg), __FILE__, __LINE__)
#define check_bytes(got, want, n, msg) course_check_bytes_((got), (want), (n), (msg), __FILE__, __LINE__)
#define check_guard(p, msg) course_check_guard_((p), (msg), __FILE__, __LINE__)

#endif
