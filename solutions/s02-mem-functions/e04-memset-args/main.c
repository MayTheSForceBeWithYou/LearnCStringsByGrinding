/* s02-e04-memset-args: harness. The defect is in slot.c, not here. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "slot.h"
#include "strcheck.h"

/* Is every byte of this slot zero? A cleared slot has no bytes of the old record in
 * it, not even in the padding and not even after the name's terminator. */
static int all_zero(const Slot *s)
{
    const unsigned char *p = (const unsigned char *)s;

    for (size_t i = 0; i < sizeof *s; i++) {
        if (p[i] != 0)
            return 0;
    }
    return 1;
}

int main(void)
{
    Slot table[SLOT_COUNT];
    Slot one;

    /* A single slot, from memory that holds something else first. */
    memset(&one, 0x5A, sizeof one);
    slot_clear(&one);
    check(all_zero(&one), "slot_clear zeroes every byte of the slot");
    check(one.id == 0, "a cleared slot has id 0");
    check_str(one.name, "", "a cleared slot has an empty name");

    /* The table, filled and then cleared and then reused. */
    memset(table, 0x5A, sizeof table);
    slot_clear_all(table, SLOT_COUNT);
    for (size_t i = 0; i < SLOT_COUNT; i++)
        check(all_zero(&table[i]), "slot_clear_all zeroes every byte of every slot");

    check(slot_set(&table[0], 7, "ada") == 0, "a short name is accepted");
    check(table[0].id == 7, "the id is stored");
    check_str(table[0].name, "ada", "the name is stored");
    check(slot_set(&table[1], 8, "bartholomew!") == -1, "a 12-character name does not fit");
    check(slot_set(&table[1], 8, "bartholome") == 0, "a 10-character name fits");

    /* The reuse that matters: clear a slot that held a long name, then write a short
     * one, and look at the bytes after the new terminator. */
    check(slot_set(&table[2], 9, "maximilien") == 0, "a 10-character name fits");
    slot_clear(&table[2]);
    check(all_zero(&table[2]), "a cleared slot keeps nothing of the record it held");
    check(slot_set(&table[2], 10, "al") == 0, "a 2-character name fits");
    check_str(table[2].name, "al", "the new name is stored");
    check_bytes(table[2].name, "al\0\0\0\0\0\0\0\0\0\0", SLOT_NAME,
                "and the rest of the name field is still zero");

    return checks_done("s02-e04-memset-args");
}
