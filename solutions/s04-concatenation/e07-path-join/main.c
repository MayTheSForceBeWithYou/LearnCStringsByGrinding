/* s04-e07-path-join: harness. Do not edit; write your code in pathjoin.c.
 *
 * Every successful call is freed. Every forced failure is checked twice: the function
 * returned NULL, and the number of live blocks is what it was before the call.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "course.h"
#include "pathjoin.h"
#include "strcheck.h"

static void one(const char *dir, const char *name, const char *want)
{
    char *got = path_join(dir, name);

    if (got == NULL) {
        fprintf(stderr, "s04-e07-path-join: out of memory\n");
        exit(EXIT_OOM);
    }
    check_str(got, want, "path_join");
    free(got);
}

static void many(const char *const *parts, size_t n, const char *want)
{
    char *got = path_join_n(parts, n);

    if (got == NULL) {
        fprintf(stderr, "s04-e07-path-join: out of memory\n");
        exit(EXIT_OOM);
    }
    check_str(got, want, "path_join_n");
    free(got);
}

int main(void)
{
    static const char *const three[] = {"usr", "local", "bin"};
    static const char *const messy[] = {"usr/", "/local/", "/bin"};
    static const char *const empties[] = {"", "a", "", "b", ""};
    size_t base = ds_live_blocks();

    /* The separator rule. */
    one("usr", "bin", "usr/bin");
    one("usr/", "bin", "usr/bin");
    one("usr", "/bin", "usr/bin");
    one("usr/", "/bin", "usr/bin");
    one("usr//", "//bin", "usr/bin");
    one("", "bin", "bin");
    one("usr", "", "usr");
    one("", "", "");
    one("/", "bin", "/bin");
    one("/", "", "/");
    one("", "/", "");
    one("a", "b", "a/b");
    one("caf\xC3\xA9", "men\xC3\xBC", "caf\xC3\xA9/men\xC3\xBC");

    /* Several parts. */
    many(three, 3, "usr/local/bin");
    many(messy, 3, "usr/local/bin");
    many(three, 1, "usr");
    many(three, 0, "");
    many(empties, 5, "a/b");

    check(ds_live_blocks() == base, "every successful join was freed");

    /* A forced failure of each of the first few allocations, one at a time. */
    for (unsigned long k = 1; k <= 4; k++) {
        size_t live = ds_live_blocks();

        ds_fail_nth(k);
        {
            char *p = path_join("usr", "bin");

            if (p != NULL) {
                check(ds_live_blocks() == live + 1, "a successful join allocated one block");
                free(p);
            } else {
                check(ds_live_blocks() == live, "a failed path_join leaves nothing allocated");
            }
        }
        ds_fail_nth(0);
    }
    for (unsigned long k = 1; k <= 6; k++) {
        size_t live = ds_live_blocks();

        ds_fail_nth(k);
        {
            char *p = path_join_n(three, 3);

            if (p != NULL) {
                check(ds_live_blocks() == live + 1, "a successful join_n allocated one block");
                free(p);
            } else {
                check(ds_live_blocks() == live, "a failed path_join_n leaves nothing allocated");
            }
        }
        ds_fail_nth(0);
    }
    check(ds_live_blocks() == base, "no block survived the forced failures");

    return checks_done("s04-e07-path-join");
}
