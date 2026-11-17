/* s03-r03-stpcpy-chain-reps: harness. Do not edit; write your code in chain.c. */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chain.h"
#include "course.h"
#include "strcheck.h"

int main(void)
{
    char *dst = guard_new(16);

    char *end = chain_two(dst, "foo", "bar");
    check(end == dst + 6, "chain_two returns a pointer to the terminator");
    check_bytes(dst, "foobar\0#########", 16, "chain_two writes a, then b, then one terminator");
    guard_reset(dst);
    check(chain_two(dst, "", "") == dst, "chain_two of two empty strings returns dst");
    check_bytes(dst, "\0###############", 16, "chain_two of two empty strings is the empty string");
    guard_reset(dst);
    check(chain_two(dst, "", "x") == dst + 1, "chain_two with an empty first part");
    check_str(dst, "x", "chain_two with an empty first part");
    guard_reset(dst);
    check(chain_two(dst, "x", "") == dst + 1, "chain_two with an empty second part");
    check_str(dst, "x", "chain_two with an empty second part");

    guard_reset(dst);
    check(chain_path(dst, "usr", "local", "bin") == 13, "chain_path returns the length of the result");
    check_bytes(dst, "usr/local/bin\0##", 16, "chain_path writes a/b/c");
    guard_reset(dst);
    check(chain_path(dst, "", "", "") == 2, "chain_path of three empty parts has length 2");
    check_str(dst, "//", "chain_path of three empty parts is two slashes");
    guard_reset(dst);
    check(chain_path(dst, "a", "", "c") == 4, "chain_path with an empty middle part");
    check_str(dst, "a//c", "chain_path copies the parts as they are");

    guard_reset(dst);
    check(chain_repeat(dst, "ab", 3) == dst + 6, "chain_repeat returns a pointer to the terminator");
    check_bytes(dst, "ababab\0#########", 16, "chain_repeat writes n copies");
    guard_reset(dst);
    check(chain_repeat(dst, "ab", 0) == dst, "chain_repeat with n == 0 returns dst");
    check_bytes(dst, "\0###############", 16, "chain_repeat with n == 0 is the empty string");
    guard_reset(dst);
    check(chain_repeat(dst, "", 5) == dst, "chain_repeat of the empty string returns dst");
    check_str(dst, "", "chain_repeat of the empty string");
    guard_free(dst);

    static const char *const parts[] = {"red", "", "green", "blue"}; /* 12 characters */
    char *small = guard_new(13);
    char *none = guard_new(0);
    check(chain_bounded(small, 13, parts, 4) == small + 12, "chain_bounded: 12 characters fit in 13 bytes");
    check_bytes(small, "redgreenblue\0", 13, "chain_bounded writes the parts in order");
    guard_reset(small);
    check(chain_bounded(small, 12, parts, 4) == NULL, "chain_bounded: 12 characters do not fit in 12 bytes");
    check(small[0] == '\0', "chain_bounded leaves the empty string when the parts do not fit");
    guard_reset(small);
    check(chain_bounded(small, 5, parts, 4) == NULL, "chain_bounded: fails in the middle of the list");
    check(small[0] == '\0', "chain_bounded leaves the empty string after failing in the middle");
    guard_reset(small);
    check(chain_bounded(small, 13, parts, 0) == small, "chain_bounded: no parts gives the empty string");
    check_bytes(small, "\0############", 13, "chain_bounded: no parts writes one terminator");
    guard_reset(small);
    check(chain_bounded(small, 1, parts, 0) == small, "chain_bounded: the empty string fits in 1 byte");
    check(chain_bounded(none, 0, parts, 0) == NULL, "chain_bounded: nothing fits in 0 bytes");
    check_guard(small, "chain_bounded stays inside cap bytes");
    check_guard(none, "chain_bounded does not touch a zero-capacity buffer");
    guard_free(none);
    guard_free(small);

    /* The point of the end pointer: appending never rescans. A million one-character
     * appends are a million steps with an end pointer, and 500 billion byte reads if
     * every append first walks the destination to find its end. */
    enum { N = 1000000 };
    char *big = guard_new(N + 1);
    clock_t t0 = clock();
    char *e = chain_repeat(big, "x", N);
    double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
    check(e == big + N && big[N - 1] == 'x' && big[N] == '\0', "chain_repeat builds a 1,000,000-character string");
    check(secs < 1.0, "chain_repeat is linear: 1,000,000 appends in under 1 s of CPU (no rescanning)");
    guard_free(big);

    return checks_done("s03-r03-stpcpy-chain-reps");
}
