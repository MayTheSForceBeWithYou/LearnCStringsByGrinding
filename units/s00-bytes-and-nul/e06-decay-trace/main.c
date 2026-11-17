/* s00-e06-decay-trace: four declarations, and what every expression over them is.
 *
 * The program prints a table of facts it can prove about the four objects: the size of
 * each expression, the offset of each address inside its own object, and which bytes
 * may be written. Fill in the template in README.md from the printout, and draw the
 * storage. The checks at the end assert the identities the table claims, so a run that
 * ends in "all checks passed" means the table you are reading is the truth.
 *
 * No address is printed. An address is meaningless on its own, differs on every run,
 * and the course never relies on %p; what matters is which addresses are equal and how
 * far apart two bytes of one object are.
 */
#include <stdio.h>
#include <string.h>

#include "course.h"

static void row(const char *expr, const char *type, size_t size, const char *note)
{
    printf("  %-14s %-16s %5zu   %s\n", expr, type, size, note);
}

int main(void)
{
    char a[] = "abc";                        /* 1: an array, sized from the literal   */
    const char *p = "abc";                   /* 2: a pointer to a literal             */
    const char *v[3] = {"a", "bb", "ccc"};   /* 3: an array of pointers               */
    char m[3][8] = {"ab", "cd", "ef"};       /* 4: an array of arrays                 */
    char *ap = a;                            /* a, decayed into a pointer variable    */

    printf("sizes and types (a pointer is %zu bytes on this machine)\n\n", sizeof(char *));
    printf("  %-14s %-16s %5s   %s\n", "expression", "its type", "sizeof", "what it denotes");
    printf("  %-14s %-16s %5s   %s\n", "----------", "--------", "------", "---------------");
    row("a", "char[4]", sizeof a, "the four bytes a b c \\0");
    row("&a[0]", "char *", sizeof &a[0], "a pointer to the first of them");
    /* `sizeof ap` and not `sizeof(a + 0)`: the two are the same number, and clang
     * rejects the second under -Wsizeof-array-decay, on the grounds that asking for the
     * size of a decayed array is usually a mistake. It is right. */
    row("ap = a", "char *", sizeof ap, "the array decayed: the same address");
    row("*a", "char", sizeof *a, "one byte: 'a'");
    row("p", "const char *", sizeof p, "the pointer variable itself");
    row("*p", "const char", sizeof *p, "one byte of the literal, 'a'");
    row("v", "const char *[3]", sizeof v, "three pointers, side by side");
    row("v[0]", "const char *", sizeof v[0], "one of them");
    row("*v[2]", "const char", sizeof *v[2], "the first byte of \"ccc\"");
    row("m", "char[3][8]", sizeof m, "three rows of eight bytes");
    row("m[1]", "char[8]", sizeof m[1], "one row (eight bytes, not a pointer)");
    row("m[1][0]", "char", sizeof m[1][0], "one byte: 'c'");

    printf("\nlengths and offsets\n\n");
    printf("  strlen(a)                 = %zu\n", strlen(a));
    printf("  strlen(p)                 = %zu\n", strlen(p));
    printf("  strlen(m[1])              = %zu\n", strlen(m[1]));
    printf("  &a[3] - &a[0]             = %td\n", &a[3] - &a[0]);
    printf("  &m[1][0] - &m[0][0]       = %td\n", &m[1][0] - &m[0][0]);
    printf("  &m[2][0] - &m[0][0]       = %td\n", &m[2][0] - &m[0][0]);

    printf("\nwhich bytes may be written\n\n");
    a[0] = 'X';
    printf("  a[0] = 'X' is fine; a is now \"%s\"\n", a);
    a[0] = 'a';
    m[1][0] = 'X';
    printf("  m[1][0] = 'X' is fine; m[1] is now \"%s\"\n", m[1]);
    m[1][0] = 'c';
    printf("  *p = 'X' is NOT fine: p points into a string literal (e03 shows what happens)\n");
    printf("  v[0] = \"z\" is fine (it is a pointer); *v[0] = 'z' is not (it is a literal)\n");

    /* The identities the table above claims. */
    check((char *)a == &a[0], "an array and the address of its first element are the same address");
    check(sizeof a == 4 && sizeof &a[0] == sizeof(char *),
          "sizeof an array is its bytes; sizeof a pointer to it is a pointer");
    check(a[0] == 'a' && a[3] == '\0', "a holds a b c and a terminator");
    check(p == &p[0], "the same identity holds for a pointer");
    check(sizeof *p == 1, "sizeof *p is one byte, whatever p points at");
    check(ap == &a[0] && sizeof ap == sizeof(char *),
          "the decayed array is the same address, and sizeof it is a pointer's");
    check(strlen(p) == 3 && sizeof p == sizeof(char *),
          "strlen(p) measures the literal; sizeof p measures the pointer");
    check(sizeof v == 3 * sizeof(char *) && sizeof v[0] == sizeof(char *),
          "an array of three pointers is three pointers wide");
    check(sizeof m == 24 && sizeof m[1] == 8 && sizeof m[1][0] == 1,
          "sizeof m, sizeof m[1] and sizeof m[1][0] are 24, 8 and 1");
    check((char *)m == &m[0][0], "m and &m[0][0] are the same address");
    check(&m[1][0] - &m[0][0] == 8, "the rows of m are eight bytes apart");
    check(strlen(m[1]) == 2 && m[1][2] == '\0' && m[1][7] == '\0',
          "a row of m holds \"cd\" and is zero to its end");

    return checks_done("s00-e06-decay-trace");
}
