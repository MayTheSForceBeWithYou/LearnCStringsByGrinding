/* lesson-demo for S00: prints the tables the lesson quotes (capture.txt), and, with an
 * argument, performs the two defects whose tool output the lesson dissects:
 *
 *     make capture TOOL=run                                    capture.txt
 *     make capture TOOL=asan ARGS=overread CAPTURE=capture-asan.txt
 *     make capture TOOL=asan ARGS=literal  CAPTURE=capture-segv.txt
 *     make capture TOOL=warn CAPTURE=capture-warn.txt
 *     make capture TOOL=warn CAPTURE=capture-warn-clang.txt CC=clang
 */
#include <stdio.h>
#include <string.h>

#include "bytes.h"

/* Through a parameter, so that the compiler has nothing to say about the copy. */
static void field_copy(char *dst, const char *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
}

static void row(const char *decl, size_t size, const char *len, const char *bytes)
{
    printf("  %-30s %6zu  %7s   %s\n", decl, size, len, bytes);
}

/* The byte 0xE9 in the three character types. */
static void signedness(void)
{
    signed char sc = (signed char)0xE9;
    unsigned char uc = (unsigned char)0xE9;
    char pc = (char)0xE9;

    printf("\nthe byte 0xE9, promoted to int\n\n");
    printf("  %-16s %6s  %8s  %9s  %8s\n", "declaration", "as int", "> 0", "< 'a'", ">> 4");
    printf("  %-16s %6s  %8s  %9s  %8s\n", "-----------", "------", "---", "-----", "----");
    printf("  %-16s %6d  %8s  %9s  %8d\n", "signed char sc", sc, sc > 0 ? "true" : "false",
           sc < 'a' ? "true" : "false", sc >> 4);
    printf("  %-16s %6d  %8s  %9s  %8d\n", "unsigned char uc", uc, uc > 0 ? "true" : "false",
           uc < 'a' ? "true" : "false", uc >> 4);
    printf("  %-16s %6d  %8s  %9s  %8d\n", "char pc", pc, pc > 0 ? "true" : "false",
           pc < 'a' ? "true" : "false", pc >> 4);
}

int main(int argc, char **argv)
{
    char a[] = "abc";
    const char *p = "abc";
    char b[3] = {'a', 'b', 'c'};
    char c[8] = "abc";
    char d[] = "ab" "cd";
    char e[] = "a\0b";
    char f[] = "";
    char r[] = "caf\xC3\xA9";
    char m[3][8] = {"ab", "cd", "ef"};

    if (argc > 1 && strcmp(argv[1], "overread") == 0) {
        char id[4];

        field_copy(id, "A123", 4);     /* four characters, four bytes, no terminator */
        printf("length: %zu\n", strlen(id));
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "literal") == 0) {
        char *w = (char *)p;           /* p points into a string literal */

        w[0] = 'A';                    /* undefined behaviour: the bytes are read-only */
        printf("%s\n", w);
        return 0;
    }

    printf("what a declaration really reserves\n\n");
    printf("  %-30s %6s  %7s   %s\n", "declaration", "sizeof", "strlen", "the bytes");
    printf("  %-30s %6s  %7s   %s\n", "-----------", "------", "------", "---------");
    row("char a[] = \"abc\";", sizeof a, "3", bytes_show(a, sizeof a));
    row("const char *p = \"abc\";", sizeof p, "3", bytes_show(p, 4));
    row("char b[3] = {'a','b','c'};", sizeof b, "-", bytes_show(b, sizeof b));
    row("char c[8] = \"abc\";", sizeof c, "3", bytes_show(c, sizeof c));
    row("char d[] = \"ab\" \"cd\";", sizeof d, "4", bytes_show(d, sizeof d));
    row("char e[] = \"a\\0b\";", sizeof e, "1", bytes_show(e, sizeof e));
    row("char f[] = \"\";", sizeof f, "0", bytes_show(f, sizeof f));
    row("char r[] = \"caf\\xC3\\xA9\";", sizeof r, "5", bytes_show(r, sizeof r));
    row("char m[3][8] = {\"ab\",...};", sizeof m, "2", bytes_show(m, sizeof m));
    printf("\n  For p, sizeof is the pointer and the bytes shown are the literal's.\n");
    printf("  For b, strlen has no answer: there is no terminator in the three bytes.\n");

    signedness();
    return 0;
}
