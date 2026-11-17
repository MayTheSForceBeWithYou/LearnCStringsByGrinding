/* roster.c -- s03-e05-unterminated-name. Fixed: see NOTES.md. */
#include "roster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Copies src into the field dst of `cap` bytes (cap > 0), cutting it to cap - 1
 * characters if it is longer. The field is always a string afterwards, and the bytes
 * after the name are NULs, so two records with the same name have the same bytes. */
static void set_field(char *dst, size_t cap, const char *src)
{
    strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

Member *member_new(int id, const char *name)
{
    Member *m = malloc(sizeof *m);
    if (m == NULL)
        return NULL;
    m->id = id;
    set_field(m->name, sizeof m->name, name);
    return m;
}

int member_name_was_cut(const Member *m, const char *original)
{
    return strlen(m->name) < strlen(original);
}

int member_label(const Member *m, char *out, size_t cap)
{
    return snprintf(out, cap, "#%d %s", m->id, m->name);
}
