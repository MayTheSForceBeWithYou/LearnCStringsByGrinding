/* lesson-demo for S01: prints the tables the lesson quotes (capture.txt), and, with an
 * argument, performs the two defects whose reports the lesson dissects:
 *
 *     make capture TOOL=run
 *     make capture TOOL=asan ARGS=wrap   CAPTURE=capture-wrap.txt
 *     make capture TOOL=asan ARGS=field  CAPTURE=capture-field.txt
 *     make capture TOOL=warn CAPTURE=capture-warn.txt
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bytes.h"

/* Through a parameter, so that the compiler has nothing to say about these two. */
static size_t len_of(const char *s)
{
    return strlen(s);
}

static size_t nlen_of(const char *s, size_t maxlen)
{
    return strnlen(s, maxlen);
}

static void row(const char *what, const char *bytes, const char *len, const char *nlen)
{
    printf("  %-26s %-22s %8s %9s\n", what, bytes, len, nlen);
}

/* Through a function, so that the comparison below is made at run time on two values
 * and not folded by the compiler (which would warn that it is always true). */
static int le(size_t a, size_t b)
{
    return a <= b;
}

static void cell(char *out, size_t cap, size_t v)
{
    snprintf(out, cap, "%zu", v);
}

int main(int argc, char **argv)
{
    static const char field4[4] = {'A', '1', '2', '3'};
    char a[] = "hello";
    char e[] = "ab\0cd";
    char f[8] = "hi";
    char l[8], n[8];

    if (argc > 1 && strcmp(argv[1], "wrap") == 0) {
        const char *s = "";
        size_t n_upper = 0;

        /* The loop of e03, on the empty string: strlen(s) - 1 is SIZE_MAX. */
        for (size_t i = 0; i <= len_of(s) - 1; i++) {
            if (s[i] >= 'A' && s[i] <= 'Z')
                n_upper++;
        }
        printf("upper: %zu\n", n_upper);
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "field") == 0) {
        printf("length: %zu\n", len_of(field4)); /* no terminator in the four bytes */
        return 0;
    }

    printf("length against size, and what each function may be asked\n\n");
    printf("  %-26s %-22s %8s %9s\n", "object", "bytes", "strlen", "strnlen");
    printf("  %-26s %-22s %8s %9s\n", "------", "-----", "------", "-------");
    cell(l, sizeof l, len_of(a));
    cell(n, sizeof n, nlen_of(a, sizeof a));
    row("char a[] = \"hello\";", bytes_show(a, sizeof a), l, n);
    cell(l, sizeof l, len_of(e));
    cell(n, sizeof n, nlen_of(e, sizeof e));
    row("char e[] = \"ab\\0cd\";", bytes_show(e, sizeof e), l, n);
    cell(l, sizeof l, len_of(f));
    cell(n, sizeof n, nlen_of(f, sizeof f));
    row("char f[8] = \"hi\";", bytes_show(f, sizeof f), l, n);
    cell(n, sizeof n, nlen_of(field4, sizeof field4));
    row("char field4[4] = {chars};", bytes_show(field4, sizeof field4), "undefined", n);
    printf("\n  strnlen is given the object's size; strlen is given nothing but the pointer.\n");

    printf("\nsize_t arithmetic at zero\n\n");
    printf("  SIZE_MAX                  = %zu\n", (size_t)SIZE_MAX);
    printf("  strlen(\"\")                = %zu\n", len_of(""));
    printf("  strlen(\"\") - 1            = %zu\n", len_of("") - 1);
    printf("  (size_t)0 - 1 == SIZE_MAX  : %s\n", (size_t)0 - 1 == SIZE_MAX ? "yes" : "no");
    printf("  0 <= strlen(\"\") - 1        : %s   (the loop that never ends)\n",
           le(0, len_of("") - 1) ? "yes" : "no");

    printf("\na length that does not fit its type\n\n");
    printf("  300 stored in unsigned char, read back  = %u\n", (unsigned)(unsigned char)300);
    printf("  256 stored in unsigned char, read back  = %u\n", (unsigned)(unsigned char)256);
    printf("  255 stored in unsigned char, read back  = %u\n", (unsigned)(unsigned char)255);
    return 0;
}
