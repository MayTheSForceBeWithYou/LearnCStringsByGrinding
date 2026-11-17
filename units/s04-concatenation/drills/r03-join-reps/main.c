/* s04-r03-join-reps: harness. Do not edit; write your code in join.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "join.h"
#include "strcheck.h"

static const char *const three[] = {"alpha", "beta", "gamma"};
static const char *const empt[] = {"", "", ""};
static const char *const mixed[] = {"a", "", "b"};

static void into_case(size_t cap, const char *const *parts, size_t n, const char *sep,
                      size_t want_ret, const char *want, const char *msg)
{
    char *dst = guard_new(cap);

    check(join_into(dst, cap, parts, n, sep) == want_ret, msg);
    if (cap > 0)
        check_str(dst, want, msg);
    check_guard(dst, "join_into writes nothing outside the buffer");
    guard_free(dst);
}

static void prefix_case(size_t cap, const char *const *parts, size_t n, const char *sep,
                        size_t want_fitted, const char *want, const char *msg)
{
    char *dst = guard_new(cap);
    size_t fitted = 999;

    join_prefix(dst, cap, parts, n, sep, &fitted);
    check(fitted == want_fitted, msg);
    if (cap > 0)
        check_str(dst, want, msg);
    check_guard(dst, "join_prefix writes nothing outside the buffer");
    guard_free(dst);
}

int main(void)
{
    /* join_len: no buffer, just the arithmetic. */
    check(join_len(three, 3, ", ") == 18, "alpha, beta, gamma is 18 characters");
    check(join_len(three, 3, "/") == 16, "with a one-character separator, 16");
    check(join_len(three, 1, "/") == 5, "one part has no separator");
    check(join_len(three, 0, "/") == 0, "no parts is the empty string");
    check(join_len(empt, 3, "/") == 2, "three empty parts are just the two separators");
    check(join_len(empt, 3, "") == 0, "with an empty separator too, nothing");
    check(join_len(mixed, 3, "-") == 4, "a-b with an empty part in the middle is a--b");

    /* join_into: the snprintf contract. */
    into_case(32, three, 3, "/", 16, "alpha/beta/gamma", "a join that fits");
    into_case(17, three, 3, "/", 16, "alpha/beta/gamma", "a join that exactly fits");
    into_case(16, three, 3, "/", 16, "alpha/beta/gamm", "a join one byte too long");
    into_case(8, three, 3, "/", 16, "alpha/b", "a join cut in the middle of a part");
    into_case(6, three, 3, "/", 16, "alpha", "a join cut at a separator");
    into_case(1, three, 3, "/", 16, "", "cap 1 holds only the terminator");
    into_case(0, three, 3, "/", 16, "", "cap 0 writes nothing");
    into_case(32, three, 1, "/", 5, "alpha", "one part");
    into_case(32, three, 0, "/", 0, "", "no parts");
    into_case(32, empt, 3, "/", 2, "//", "three empty parts");
    into_case(32, mixed, 3, "-", 4, "a--b", "an empty part in the middle");
    into_case(32, three, 3, ", ", 18, "alpha, beta, gamma", "a two-character separator");

    /* join_prefix: whole parts only. */
    prefix_case(32, three, 3, "/", 3, "alpha/beta/gamma", "everything fits");
    prefix_case(17, three, 3, "/", 3, "alpha/beta/gamma", "everything exactly fits");
    prefix_case(16, three, 3, "/", 2, "alpha/beta", "the last part does not fit");
    prefix_case(11, three, 3, "/", 2, "alpha/beta", "two parts exactly fit");
    prefix_case(10, three, 3, "/", 1, "alpha", "only the first part fits");
    prefix_case(6, three, 3, "/", 1, "alpha", "the first part fits with nothing to spare");
    prefix_case(5, three, 3, "/", 0, "", "not even the first part fits");
    prefix_case(1, three, 3, "/", 0, "", "cap 1");
    prefix_case(32, three, 0, "/", 0, "", "no parts");

    /* The return value of join_prefix describes what it wrote. */
    {
        char *dst = guard_new(16);
        size_t fitted = 0;
        size_t r = join_prefix(dst, 16, three, 3, "/", &fitted);

        check(fitted == 2 && r == 10, "join_prefix reports the length it actually wrote");
        check(r < 16, "which fits, by construction");
        check(join_prefix(dst, 16, three, 3, "/", NULL) == 10, "fitted may be NULL");
        guard_free(dst);
    }

    return checks_done("s04-r03-join-reps");
}
