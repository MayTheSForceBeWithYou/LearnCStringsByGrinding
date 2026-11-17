/* lesson-demo for S05: the four reports the lesson quotes, each from one run.
 *
 *     make capture TOOL=asan ARGS=stack  CAPTURE=capture-stack.txt
 *     make capture TOOL=asan ARGS=heap   CAPTURE=capture-heap.txt
 *     make capture TOOL=asan ARGS=global CAPTURE=capture-global.txt
 *     make capture TOOL=run  ARGS=quiet  CAPTURE=capture-quiet.txt
 *     make capture TOOL=warn CAPTURE=capture-warn.txt
 *     make capture TOOL=warn CAPTURE=capture-warn-clang.txt CC=clang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"

static const char months[3][4] = {"Jan", "Feb", "Mar"};

typedef struct {
    char user[8];
    char role[8];
} Account;

/* Through parameters, so that the compiler says nothing about the calls. */
static void copy_into(char *dst, const char *src)
{
    strcpy(dst, src);
}

static void copy_n(char *dst, const char *src, size_t n)
{
    memcpy(dst, src, n);
}

static const char *month_of(int m)
{
    return months[m - 1];
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";

    if (strcmp(what, "stack") == 0) {
        char tag[8];

        copy_into(tag, "Dodecahedron"); /* 12 characters into 8 bytes */
        puts(tag);
        return 0;
    }
    if (strcmp(what, "heap") == 0) {
        char *code = malloc(6);

        if (code == NULL)
            return 3;
        copy_n(code, "AB1234", 6); /* six characters, no terminator */
        printf("%s\n", code);      /* the read runs off the block */
        free(code);
        return 0;
    }
    if (strcmp(what, "global") == 0) {
        int m = (int)strlen(what) - 2; /* 4, which the table does not have */

        printf("%s\n", month_of(m));
        return 0;
    }
    if (strcmp(what, "quiet") == 0) {
        Account a;

        memset(&a, 0, sizeof a);
        copy_into(a.role, "guest");
        printf("before: %s\n", bytes_show(&a, sizeof a));
        copy_into(a.user, "mallory_admin"); /* 13 characters into 8 bytes */
        printf("after:  %s\n", bytes_show(&a, sizeof a));
        printf("user:   %s\n", a.user);
        printf("role:   %s\n", a.role);
        printf("is admin: %s\n", strcmp(a.role, "admin") == 0 ? "yes" : "no");
        return 0;
    }

    puts("usage: prog stack | heap | global | quiet");
    return 0;
}
