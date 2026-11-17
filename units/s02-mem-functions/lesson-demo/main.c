/* lesson-demo for S02: prints the tables the lesson quotes (capture.txt), and, with an
 * argument, performs the defect whose report the lesson dissects:
 *
 *     make capture TOOL=run
 *     make capture TOOL=asan ARGS=overlap CAPTURE=capture-overlap.txt
 *     make capture TOOL=warn CAPTURE=capture-warn.txt
 *     make capture TOOL=warn CAPTURE=capture-warn-clang.txt CC=clang
 */
#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "strcheck.h"

enum { BUF = 8 };

typedef struct {
    char kind;
    int value;
} Point;

/* Through a parameter, so that the compiler has nothing to say about these. */
static void fill(char *p, int c, size_t n)
{
    memset(p, c, n);
}

static void *copy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static void row(const char *call, const char *buf, const char *note)
{
    printf("  %-32s %-20s %s\n", call, buf, note);
}

static void reset(char *buf)
{
    memset(buf, '#', BUF);
}

int main(int argc, char **argv)
{
    char buf[BUF];
    char note[64];

    if (argc > 1 && strcmp(argv[1], "overlap") == 0) {
        char text[32] = "abcdefghijklmnopqrstuvwxyz";

        copy(text, text + 4, 23); /* the two areas overlap: undefined */
        printf("%s\n", text);
        return 0;
    }

    printf("what each call leaves in an 8-byte buffer holding ########\n\n");
    printf("  %-32s %-20s %s\n", "call", "buffer after", "note");
    printf("  %-32s %-20s %s\n", "----", "------------", "----");
    reset(buf);
    fill(buf, 'x', 3);
    row("memset(buf, 'x', 3)", bytes_show(buf, BUF), "three bytes, the rest untouched");
    reset(buf);
    fill(buf, 0x141, 4);
    row("memset(buf, 0x141, 4)", bytes_show(buf, BUF), "0x141 converted to unsigned char: 'A'");
    reset(buf);
    fill(buf, -1, 2);
    row("memset(buf, -1, 2)", bytes_show(buf, BUF), "and -1 becomes 0xFF");
    reset(buf);
    fill(buf, 'x', 0);
    row("memset(buf, 'x', 0)", bytes_show(buf, BUF), "n == 0 writes nothing");

    memcpy(buf, "abcdefgh", BUF);
    memmove(buf + 2, buf, 6);
    row("memmove(buf + 2, buf, 6)", bytes_show(buf, BUF), "dst above src: copied from the end");
    memcpy(buf, "abcdefgh", BUF);
    memmove(buf, buf + 2, 6);
    row("memmove(buf, buf + 2, 6)", bytes_show(buf, BUF), "dst below src: copied from the start");
    memcpy(buf, "abcdefgh", BUF);
    memmove(buf, buf, BUF);
    row("memmove(buf, buf, 8)", bytes_show(buf, BUF), "dst == src: nothing changes");

    printf("\nmemcmp compares bytes as unsigned char, and promises only a sign\n\n");
    printf("  %-28s %6s   %s\n", "comparison", "sign", "why");
    printf("  %-28s %6s   %s\n", "----------", "----", "---");
    printf("  %-28s %6d   %s\n", "memcmp(\"abc\", \"abc\", 3)",
           sign_of(memcmp("abc", "abc", 3)), "equal");
    printf("  %-28s %6d   %s\n", "memcmp(\"abc\", \"abd\", 3)",
           sign_of(memcmp("abc", "abd", 3)), "'c' < 'd'");
    printf("  %-28s %6d   %s\n", "memcmp(\"abc\", \"abd\", 2)",
           sign_of(memcmp("abc", "abd", 2)), "the difference is past n");
    printf("  %-28s %6d   %s\n", "memcmp(\"\\x7F\", \"\\x80\", 1)",
           sign_of(memcmp("\x7F", "\x80", 1)), "0x7F < 0x80 as unsigned char");
    printf("  %-28s %6d   %s\n", "memcmp(\"\\xFF\", \"\\x01\", 1)",
           sign_of(memcmp("\xFF", "\x01", 1)), "0xFF > 0x01, not negative");
    printf("  %-28s %6d   %s\n", "memcmp(a, b, 0)",
           sign_of(memcmp("a", "b", 0)), "no bytes compared");

    printf("\na struct has bytes that are not members\n\n");
    {
        Point p, q;
        unsigned char *pp = (unsigned char *)&p;

        memset(&p, 0x00, sizeof p);
        memset(&q, 0x5A, sizeof q);
        p.kind = 'p';
        p.value = 42;
        q.kind = 'p';
        q.value = 42;
        printf("  sizeof(Point)                = %zu  (char + int is %zu bytes of members)\n",
               sizeof(Point), sizeof(char) + sizeof(int));
        printf("  p, padding zeroed            = %s\n", bytes_show(&p, sizeof p));
        printf("  q, padding 0x5A              = %s\n", bytes_show(&q, sizeof q));
        printf("  p.kind == q.kind && p.value == q.value : %s\n",
               p.kind == q.kind && p.value == q.value ? "yes" : "no");
        snprintf(note, sizeof note, "%s", memcmp(&p, &q, sizeof p) == 0 ? "equal" : "NOT equal");
        printf("  memcmp(&p, &q, sizeof p)               : %s\n", note);
        (void)pp;
    }
    return 0;
}
