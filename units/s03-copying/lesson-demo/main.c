/* lesson-demo for S03: prints the byte diagrams the lesson quotes (capture.txt), and,
 * with the argument "overread", performs the over-read whose ASan report the lesson
 * dissects (capture-asan.txt). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"

enum { BUF = 8 };

static void show(const char *call, const char *buf, const char *note)
{
    printf("%-38s", call);
    for (int i = 0; i < BUF; i++)
        printf(" %-3s", bytes_show(buf + i, 1));
    printf("  %s\n", note);
}

static void reset(char *buf)
{
    memset(buf, '#', BUF);
}

/* Through a function, so that the compiler does not comment on the calls below. */
static char *field_copy(char *dst, const char *src, size_t n)
{
    return strncpy(dst, src, n);
}

static int fmt_copy(char *dst, size_t cap, const char *src)
{
    return snprintf(dst, cap, "%s", src);
}

struct member {
    int id;
    char name[12];
};

int main(int argc, char **argv)
{
    char buf[BUF];
    char note[64];

    printf("%-38s", "index:");
    for (int i = 0; i < BUF; i++)
        printf(" %-3d", i);
    printf("\n");

    reset(buf);
    show("before every call", buf, "");

    reset(buf);
    strcpy(buf, "hello");
    show("strcpy(buf, \"hello\")", buf, "returns buf");

    reset(buf);
    snprintf(note, sizeof note, "returns buf + %td", stpcpy(buf, "hello") - buf);
    show("stpcpy(buf, \"hello\")", buf, note);

    reset(buf);
    field_copy(buf, "hello", 8);
    show("strncpy(buf, \"hello\", 8)", buf, "terminated: yes (5 < 8, padded)");

    reset(buf);
    field_copy(buf, "hello", 5);
    show("strncpy(buf, \"hello\", 5)", buf, "terminated: NO (5 == n)");

    reset(buf);
    field_copy(buf, "hello", 3);
    show("strncpy(buf, \"hello\", 3)", buf, "terminated: NO (5 > n)");

    reset(buf);
    snprintf(note, sizeof note, "returns buf + %td", stpncpy(buf, "hey", 8) - buf);
    show("stpncpy(buf, \"hey\", 8)", buf, note);

    reset(buf);
    snprintf(note, sizeof note, "returns buf + %td (== buf + n)", stpncpy(buf, "abcdefgh", 8) - buf);
    show("stpncpy(buf, \"abcdefgh\", 8)", buf, note);

    reset(buf);
    snprintf(note, sizeof note, "returns %d", fmt_copy(buf, 8, "hello"));
    show("snprintf(buf, 8, \"%s\", \"hello\")", buf, note);

    reset(buf);
    snprintf(note, sizeof note, "returns %d (>= 8: truncated)", fmt_copy(buf, 8, "abcdefghij"));
    show("snprintf(buf, 8, \"%s\", \"abcdefghij\")", buf, note);

    if (argc > 1 && strcmp(argv[1], "overread") == 0) {
        struct member *m = malloc(sizeof *m);
        if (m == NULL)
            return 3;
        m->id = 7;
        field_copy(m->name, "Maximilienne", sizeof m->name); /* 12 characters, 12 bytes */
        printf("name length: %zu\n", strlen(m->name));       /* reads past the record */
        free(m);
    }
    return 0;
}
