/* roster.h -- s03-e05-unterminated-name: club members with a fixed-size name field. */
#ifndef ROSTER_H
#define ROSTER_H

#include <stddef.h>

#define NAME_SIZE 12 /* bytes in the name field: at most 11 characters and a terminator */

typedef struct {
    int id;
    char name[NAME_SIZE]; /* always a string: terminated within the field */
} Member;

/* A new heap-allocated member, or NULL if allocation fails. `name` is borrowed and
 * copied into the record; a name longer than NAME_SIZE - 1 characters is cut to its
 * first NAME_SIZE - 1 characters. The caller frees the member with free(). */
Member *member_new(int id, const char *name);

/* 1 if the name given to member_new had to be cut to fit, else 0. */
int member_name_was_cut(const Member *m, const char *original);

/* Writes "#<id> <name>" to out (capacity cap) and returns the length it needed,
 * like snprintf. */
int member_label(const Member *m, char *out, size_t cap);

#endif
