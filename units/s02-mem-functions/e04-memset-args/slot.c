/* slot.c -- s02-e04-memset-args. There is one defect in this file. */
#include "slot.h"

#include <string.h>

/* Sets `size` bytes at p to the byte `value`. */
static void fill_bytes(void *p, size_t size, int value)
{
    memset(p, size, value);
}

void slot_clear(Slot *s)
{
    fill_bytes(s, sizeof *s, 0);
}

void slot_clear_all(Slot *table, size_t n)
{
    for (size_t i = 0; i < n; i++)
        slot_clear(&table[i]);
}

int slot_set(Slot *s, int id, const char *name)
{
    size_t len = strlen(name);

    if (len + 1 > SLOT_NAME)
        return -1;
    s->id = id;
    memcpy(s->name, name, len + 1);
    return 0;
}
