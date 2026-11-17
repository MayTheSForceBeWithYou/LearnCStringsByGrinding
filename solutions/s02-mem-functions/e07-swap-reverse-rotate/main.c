/* s02-e07-swap-reverse-rotate: harness. Do not edit; write your code in memops.c. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "memops.h"
#include "oracle.h"
#include "strcheck.h"

/* An array of n elements of `size` bytes, element i filled with the byte 'a' + i. */
static char *make_array(size_t n, size_t size)
{
    char *p = guard_new(n * size);

    for (size_t i = 0; i < n; i++)
        memset(p + i * size, (int)('a' + (int)i), size);
    return p;
}

/* The same array with its elements in the given order, as a string of one letter per
 * element repeated `size` times -- what the array should look like afterwards. */
static void expect(char *out, const char *order, size_t size)
{
    size_t at = 0;

    for (size_t i = 0; order[i] != '\0'; i++) {
        memset(out + at, order[i], size);
        at += size;
    }
    out[at] = '\0';
}

static void reverse_case(size_t n, size_t size, const char *order, const char *msg)
{
    char *a = make_array(n, size);
    char want[256];

    expect(want, order, size);
    mem_reverse(a, n, size);
    check_bytes(a, want, n * size, msg);
    check_guard(a, "mem_reverse writes nothing outside the array");
    guard_free(a);
}

static void rotate_case(size_t n, size_t size, size_t k, const char *order, const char *msg)
{
    char *a = make_array(n, size);
    char want[256];

    expect(want, order, size);
    mem_rotate_left(a, n, size, k);
    check_bytes(a, want, n * size, msg);
    check_guard(a, "mem_rotate_left writes nothing outside the array");
    guard_free(a);
}

/* The slow, obvious rotation, for comparison on generated cases. */
static void ref_rotate(char *dst, const char *src, size_t n, size_t size, size_t k)
{
    for (size_t i = 0; i < n; i++)
        memcpy(dst + i * size, src + ((i + k) % n) * size, size);
}

static void oracle(void)
{
    enum { CASES = 600 };
    int ok = 1;

    orc_seed(207);
    for (int c = 0; c < CASES && ok; c++) {
        size_t n = 1 + orc_below(9);
        size_t size = 1 + orc_below(40);
        size_t k = orc_below(20);
        char *a = make_array(n, size);
        char ref[9 * 40], before[9 * 40];

        memcpy(before, a, n * size);
        ref_rotate(ref, before, n, size, k % n);
        mem_rotate_left(a, n, size, k);
        if (memcmp(a, ref, n * size) != 0) {
            ok = 0;
            printf("       mem_rotate_left differs: seed 207, case %d, n = %zu, size = %zu, k = %zu\n",
                   c, n, size, k);
            printf("       yours: %s\n", bytes_show(a, n * size));
            printf("       want:  %s\n", bytes_show(ref, n * size));
        }
        guard_free(a);
    }
    check(ok, "mem_rotate_left agrees with an element-by-element rotation on 600 generated cases");
}

int main(void)
{
    /* mem_swap, including elements larger than the scratch buffer. */
    {
        char *a = guard_copy("aaaa", 4);
        char *b = guard_copy("bbbb", 4);

        mem_swap(a, b, 4);
        check_bytes(a, "bbbb", 4, "mem_swap exchanges the bytes");
        check_bytes(b, "aaaa", 4, "mem_swap exchanges both ways");
        mem_swap(a, a, 4);
        check_bytes(a, "bbbb", 4, "mem_swap with a == b changes nothing");
        mem_swap(a, b, 0);
        check_bytes(a, "bbbb", 4, "mem_swap with size 0 changes nothing");
        guard_free(b);
        guard_free(a);
    }
    {   /* 100 bytes each: more than MEMOPS_CHUNK, so it must move in pieces. */
        char *a = guard_new(100);
        char *b = guard_new(100);
        char wa[101], wb[101];

        memset(a, 'A', 100);
        memset(b, 'B', 100);
        memset(wa, 'B', 100);
        memset(wb, 'A', 100);
        mem_swap(a, b, 100);
        check_bytes(a, wa, 100, "mem_swap exchanges elements larger than the scratch buffer");
        check_bytes(b, wb, 100, "and both of them");
        check_guard(a, "mem_swap writes nothing outside a");
        check_guard(b, "mem_swap writes nothing outside b");
        guard_free(b);
        guard_free(a);
    }

    /* mem_reverse */
    reverse_case(5, 1, "edcba", "mem_reverse of five one-byte elements");
    reverse_case(4, 1, "dcba", "mem_reverse of an even number of elements");
    reverse_case(1, 1, "a", "mem_reverse of one element");
    reverse_case(0, 1, "", "mem_reverse of no elements");
    reverse_case(3, 4, "cba", "mem_reverse of three four-byte elements");
    reverse_case(2, 33, "ba", "mem_reverse of elements larger than the scratch buffer");
    reverse_case(5, 7, "edcba", "mem_reverse of five seven-byte elements");

    /* mem_rotate_left */
    rotate_case(5, 1, 0, "abcde", "rotating by 0 changes nothing");
    rotate_case(5, 1, 1, "bcdea", "rotating by 1");
    rotate_case(5, 1, 2, "cdeab", "rotating by 2");
    rotate_case(5, 1, 4, "eabcd", "rotating by n - 1");
    rotate_case(5, 1, 5, "abcde", "rotating by n changes nothing");
    rotate_case(5, 1, 7, "cdeab", "rotating by more than n is rotating by k % n");
    rotate_case(6, 1, 3, "defabc", "rotating by half");
    rotate_case(6, 1, 2, "cdefab", "rotating six elements by two");
    rotate_case(1, 1, 3, "a", "rotating one element");
    rotate_case(0, 1, 3, "", "rotating no elements");
    rotate_case(4, 5, 1, "bcda", "rotating four five-byte elements");
    rotate_case(3, 40, 2, "cab", "rotating elements larger than the scratch buffer");

    oracle();
    return checks_done("s02-e07-swap-reverse-rotate");
}
