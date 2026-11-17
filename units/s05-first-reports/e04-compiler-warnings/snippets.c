/* snippets.c -- s05-e04-compiler-warnings: ten small functions, each one a line of C
 * that a compiler may or may not have something to say about.
 *
 * This file is never linked and never run. The harness does not compile it either;
 * `make warn` does, at -O2 with -Wall -Wextra -Wpedantic, and that is the question:
 * which of the ten draw a diagnostic, and under which flag.
 *
 * Predict first. Fill in answers.txt, then run `make check`. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 01 */
void s01_pad(char *dst, const char *src)
{
    strncpy(dst, src, 8);
}

/* 02 */
void s02_pad_known(void)
{
    char dst[8];

    strncpy(dst, "abcdefgh", sizeof dst);
    puts(dst);
}

/* 03 */
size_t s03_sizeof_pointer(const char *p)
{
    return sizeof p;
}

/* 04 */
void s04_clear(char *p)
{
    memset(p, 0, sizeof p);
}

/* 05 */
int s05_format(const char *name, int n)
{
    return printf("%s has %s items\n", name, (const char *)&n);
}

/* 06 */
int s06_wrong_specifier(size_t n)
{
    return printf("%d\n", n);
}

/* 07 */
void s07_literal_into_small(void)
{
    char small[8];

    strcpy(small, "a much longer greeting");
    puts(small);
}

/* 08 */
int s08_truncating_format(const char *name)
{
    char label[8];

    return snprintf(label, sizeof label, "user: %s", name);
}

/* 09 */
void s09_unterminated_init(void)
{
    char code[3] = "abc";

    puts(code);
}

/* 10 */
size_t s10_strlen_of_field(void)
{
    static const char id[4] = {'A', '1', '2', '3'};

    return strlen(id);
}
