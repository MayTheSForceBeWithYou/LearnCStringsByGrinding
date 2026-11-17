/* lesson-demo for S04: prints the tables the lesson quotes (capture.txt), and with an
 * argument performs the overflow whose report the lesson dissects:
 *
 *     make capture TOOL=run
 *     make capture TOOL=asan ARGS=overflow CAPTURE=capture-overflow.txt
 *     make capture TOOL=warn CAPTURE=capture-warn.txt
 *     make capture TOOL=warn CAPTURE=capture-warn-clang.txt CC=clang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bytes.h"

enum { BUF = 8 };

/* Through parameters, so that the compiler has nothing to say about the calls. */
static char *cat_n(char *dst, const char *src, size_t n)
{
    return strncat(dst, src, n);
}

static int fmt(char *dst, size_t cap, const char *a, const char *b)
{
    return snprintf(dst, cap, "%s%s", a, b);
}

/* The strlcat contract, written out because the course cannot call strlcat. */
static size_t lcat(char *dst, const char *src, size_t size)
{
    size_t dlen = strnlen(dst, size), slen = strlen(src);

    if (dlen == size)
        return size + slen;
    for (size_t i = 0; i < slen && dlen + i + 1 < size; i++)
        dst[dlen + i] = src[i];
    {
        size_t room = size - dlen - 1;
        size_t keep = slen < room ? slen : room;

        dst[dlen + keep] = '\0';
    }
    return dlen + slen;
}

/* The 8-byte buffer, filled with '#' and then holding "ab", before every call. */
static void reset(char *buf)
{
    memset(buf, '#', BUF);
    memcpy(buf, "ab", 3);
}

static void row(const char *call, const char *buf, const char *ret, const char *note)
{
    printf("  %-34s %-14s %6s  %s\n", call, buf, ret, note);
}

static double cpu_since(clock_t t0)
{
    return (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
}

int main(int argc, char **argv)
{
    char buf[BUF], note[32];

    if (argc > 1 && strcmp(argv[1], "overflow") == 0) {
        char dst[8] = "abcdef";

        cat_n(dst, "ghij", sizeof dst); /* the n of strncat is not the destination's size */
        puts(dst);
        return 0;
    }

    printf("appending \"cd\" and \"cdefghij\" to \"ab\" in an 8-byte buffer\n\n");
    printf("  %-34s %-14s %6s  %s\n", "call", "buffer after", "ret", "note");
    printf("  %-34s %-14s %6s  %s\n", "----", "------------", "---", "----");

    reset(buf);
    strcat(buf, "cd");
    row("strcat(buf, \"cd\")", bytes_show(buf, BUF), "buf", "no bound at all");

    reset(buf);
    cat_n(buf, "cdefghij", 3);
    row("strncat(buf, \"cdefghij\", 3)", bytes_show(buf, BUF), "buf", "3 characters, then a terminator");

    reset(buf);
    cat_n(buf, "cdefghij", 5);
    row("strncat(buf, \"cdefghij\", 5)", bytes_show(buf, BUF), "buf", "5 characters: the buffer is full");

    reset(buf);
    snprintf(note, sizeof note, "%d", fmt(buf, BUF, "ab", "cd"));
    row("snprintf(buf, 8, \"%s%s\", ...)", bytes_show(buf, BUF), note, "rebuilds; returns the length");

    reset(buf);
    snprintf(note, sizeof note, "%d", fmt(buf, BUF, "ab", "cdefghij"));
    row("snprintf(buf, 8, ... too long)", bytes_show(buf, BUF), note, "truncated: ret >= 8");

    reset(buf);
    snprintf(note, sizeof note, "%zu", lcat(buf, "cd", BUF));
    row("strlcat-style append of \"cd\"", bytes_show(buf, BUF), note, "appends; returns the length");

    reset(buf);
    snprintf(note, sizeof note, "%zu", lcat(buf, "cdefghij", BUF));
    row("strlcat-style, too long", bytes_show(buf, BUF), note, "truncated: ret >= 8");

    printf("\nwhat it costs to append n words of 4 characters, two ways\n\n");
    printf("  %10s  %14s  %14s  %s\n", "words", "strcat loop", "end pointer", "ratio");
    printf("  %10s  %14s  %14s  %s\n", "-----", "-----------", "-----------", "-----");
    for (size_t words = 10000; words <= 80000; words *= 2) {
        size_t cap = words * 5 + 1;
        char *dst = malloc(cap);
        double t_cat, t_end;

        if (dst == NULL)
            return 3;
        {
            clock_t t0 = clock();

            dst[0] = '\0';
            for (size_t i = 0; i < words; i++)
                strcat(dst, "abcd");
            t_cat = cpu_since(t0);
        }
        {
            clock_t t0 = clock();
            size_t at = 0;

            dst[0] = '\0';
            for (size_t i = 0; i < words; i++) {
                memcpy(dst + at, "abcd", 4);
                at += 4;
                dst[at] = '\0';
            }
            t_end = cpu_since(t0);
        }
        printf("  %10zu  %12.4f s  %12.4f s  %6.0fx\n", words, t_cat, t_end,
               t_end > 0 ? t_cat / t_end : 0.0);
        free(dst);
    }
    return 0;
}
