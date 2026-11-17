/* s03-e05-unterminated-name: harness. The defect is in roster.c, not here. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "course.h"
#include "roster.h"
#include "strcheck.h"

static void one(int id, const char *name, const char *want_name, int want_cut, const char *want_label)
{
    char label[64];
    Member *m = member_new(id, name);
    if (m == NULL) {
        fprintf(stderr, "s03-e05-unterminated-name: out of memory\n");
        exit(EXIT_OOM);
    }
    check(memchr(m->name, '\0', NAME_SIZE) != NULL, "the name field holds a terminator");
    check(strlen(m->name) == strlen(want_name), "the stored name has the expected length");
    check_str(m->name, want_name, "the stored name");
    check(member_name_was_cut(m, name) == want_cut, "member_name_was_cut");
    member_label(m, label, sizeof label);
    check_str(label, want_label, "member_label");
    free(m);
}

int main(void)
{
    check(sizeof(Member) == sizeof(int) + NAME_SIZE, "the name field is the last 12 bytes of the record");

    one(1, "Ada", "Ada", 0, "#1 Ada");
    one(2, "", "", 0, "#2 ");
    one(3, "Grace", "Grace", 0, "#3 Grace");
    one(4, "Bartholomew", "Bartholomew", 0, "#4 Bartholomew");            /* 11: the longest that fits */
    one(5, "Maximilienne", "Maximilienn", 1, "#5 Maximilienn");           /* 12: one too many */
    one(6, "Wolfeschlegelstein", "Wolfeschleg", 1, "#6 Wolfeschleg");     /* 18 */

    return checks_done("s03-e05-unterminated-name");
}
