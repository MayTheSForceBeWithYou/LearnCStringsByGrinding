/* room.c -- s04-r02-room-math-reps: your four functions. */
#include "room.h"

#include <stdint.h>

/* ---- Your definitions of the room.h functions go below this line. ---- */

size_t room_left(size_t cap, size_t len)
{
    if (cap == 0 || len + 1 >= cap)
        return 0;
    return cap - len - 1;
}

int will_fit(size_t cap, size_t len, size_t add)
{
    if (cap == 0 || len >= cap)
        return 0;
    return add <= cap - len - 1;
}

size_t size_needed(size_t a, size_t b)
{
    if (b > SIZE_MAX - a)
        return 0;
    if (a + b > SIZE_MAX - 1)
        return 0;
    return a + b + 1;
}

size_t size_needed3(size_t a, size_t b, size_t c)
{
    /* The two separators first, then the three parts: a + 2 + b, and then
     * size_needed adds c and the terminator. Each step checks for a wrap before it
     * happens, so no sum is ever computed that could not be represented. */
    if (a > SIZE_MAX - 2)
        return 0;
    a += 2;
    if (b > SIZE_MAX - a)
        return 0;
    return size_needed(a + b, c);
}
