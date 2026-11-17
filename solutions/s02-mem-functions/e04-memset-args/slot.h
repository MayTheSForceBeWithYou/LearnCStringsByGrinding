/* slot.h -- s02-e04-memset-args: a table of fixed-size records that is reused. */
#ifndef SLOT_H
#define SLOT_H

#include <stddef.h>

#define SLOT_NAME 12
#define SLOT_COUNT 4

typedef struct {
    int id;
    char name[SLOT_NAME];
} Slot;

/* Clears one slot: every byte of it becomes 0, so the id is 0 and the name is the
 * empty string with no bytes of the previous record left in it. */
void slot_clear(Slot *s);

/* Clears the whole table: n slots, every byte 0. */
void slot_clear_all(Slot *table, size_t n);

/* Fills a cleared slot. `name` is borrowed and must fit in SLOT_NAME - 1 characters;
 * returns 0, or -1 (changing nothing) if it does not. */
int slot_set(Slot *s, int id, const char *name);

#endif
