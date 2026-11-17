/* course.h -- the tiny shared test harness for LearnCStringsByGrinding.
 *
 *     check(n == 5, "five bytes copied");
 *     ...
 *     return checks_done("s03-e01-my-strcpy-stpcpy");
 *
 * check() prints a FAIL line (to stdout) and counts the failure; it never aborts.
 * checks_done() prints "<id>: all checks passed" and returns 0, or prints the
 * failure count and returns 1. Return its value from main.
 *
 * Exit codes every program in the course uses:
 *   0          all checks passed
 *   1          at least one check failed
 *   3          EXIT_OOM: an allocation failed and the program cleaned up and stopped.
 *              Free everything you own, print one line to stderr, return EXIT_OOM.
 *   4          EXIT_LEAK: set by lib/dsalloc.c at exit when blocks are still live.
 *   5          EXIT_NO_LOCALE: no UTF-8 locale on this machine (lib/utf8locale.c says how to fix it)
 *   124        killed by scripts/run-timeout.sh (time limit)
 *   128 + sig  killed by a signal (134 = SIGABRT, which is how ASan/UBSan stop)
 *
 * lib/dsalloc.h is force-included before this file, so its query functions
 * (ds_live_blocks, ds_fail_nth, ...) are available to every harness.
 */
#ifndef COURSE_H
#define COURSE_H

#include <stdio.h>

#define EXIT_OOM 3
#define EXIT_LEAK 4
#define EXIT_NO_LOCALE 5

static inline int *course_failures_(void)
{
    static int failures;
    return &failures;
}

static inline void course_check_(int ok, const char *msg, const char *file, int line)
{
    if (!ok) {
        printf("FAIL %s:%d: %s\n", file, line, msg);
        fflush(stdout);
        ++*course_failures_();
    }
}

#define check(cond, msg) course_check_((cond) ? 1 : 0, (msg), __FILE__, __LINE__)

static inline int checks_done(const char *id)
{
    int n = *course_failures_();
    if (n == 0) {
        printf("%s: all checks passed\n", id);
        return 0;
    }
    printf("%s: %d check(s) failed\n", id, n);
    return 1;
}

#endif
