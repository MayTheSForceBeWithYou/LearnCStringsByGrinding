/* s04-r02-room-math-reps: harness. Do not edit; write your code in room.c.
 *
 * Nothing here touches a buffer: these four are arithmetic, and the cases that matter
 * are the extremes of size_t.
 */
#include <stdint.h>
#include <stdio.h>

#include "course.h"
#include "room.h"

int main(void)
{
    /* room_left */
    check(room_left(8, 0) == 7, "an empty 8-byte buffer has room for 7");
    check(room_left(8, 3) == 4, "three characters in, room for 4");
    check(room_left(8, 7) == 0, "a full buffer has room for none");
    check(room_left(8, 8) == 0, "len == cap: no room, and no wrap");
    check(room_left(8, 9) == 0, "len larger than cap: no room, and no wrap");
    check(room_left(1, 0) == 0, "a one-byte buffer holds only a terminator");
    check(room_left(0, 0) == 0, "a zero-byte buffer has no room");
    check(room_left(0, 5) == 0, "and still none when len is nonsense");
    check(room_left(SIZE_MAX, 0) == SIZE_MAX - 1, "the largest buffer");

    /* will_fit */
    check(will_fit(8, 0, 7) == 1, "7 characters fit in 8 bytes");
    check(will_fit(8, 0, 8) == 0, "8 do not");
    check(will_fit(8, 3, 4) == 1, "3 + 4 fit in 8");
    check(will_fit(8, 3, 5) == 0, "3 + 5 do not");
    check(will_fit(8, 0, 0) == 1, "nothing always fits, if there is a byte for the terminator");
    check(will_fit(0, 0, 0) == 0, "nothing fits in no bytes, not even nothing");
    check(will_fit(1, 0, 0) == 1, "the empty string fits in one byte");
    check(will_fit(8, 8, 0) == 0, "len == cap leaves no room for a terminator");
    check(will_fit(8, 2, SIZE_MAX) == 0, "an addition that would wrap does not fit");
    check(will_fit(SIZE_MAX, SIZE_MAX - 2, 1) == 1, "the largest append that fits");
    check(will_fit(SIZE_MAX, SIZE_MAX - 2, 2) == 0, "and one more that does not");

    /* size_needed */
    check(size_needed(3, 4) == 8, "3 + 4 characters need 8 bytes");
    check(size_needed(0, 0) == 1, "the empty string needs 1 byte");
    check(size_needed(5, 0) == 6, "one part only");
    check(size_needed(SIZE_MAX - 1, 0) == SIZE_MAX, "the largest pair that fits in a size_t");
    check(size_needed(SIZE_MAX, 0) == 0, "a + 1 would wrap: reported as 0");
    check(size_needed(SIZE_MAX, 1) == 0, "a + b would wrap: reported as 0");
    check(size_needed(SIZE_MAX / 2 + 1, SIZE_MAX / 2 + 1) == 0, "and so would this pair");

    /* size_needed3: three parts and two separators */
    check(size_needed3(3, 4, 5) == 15, "3 + 1 + 4 + 1 + 5 + 1 is 15");
    check(size_needed3(0, 0, 0) == 3, "three empty parts still need two separators");
    check(size_needed3(1, 1, 1) == 6, "1 + 1 + 1 + two separators + terminator");
    check(size_needed3(SIZE_MAX, 0, 0) == 0, "a sum that would wrap is reported as 0");
    check(size_needed3(SIZE_MAX - 3, 1, 1) == 0, "and one that wraps only at the end");
    check(size_needed3(SIZE_MAX / 3, SIZE_MAX / 3, SIZE_MAX / 3) == 0,
          "three parts of SIZE_MAX / 3 need three bytes more than a size_t holds");
    check(size_needed3(SIZE_MAX / 4, SIZE_MAX / 4, SIZE_MAX / 4) == SIZE_MAX / 4 * 3 + 3,
          "three large parts that do still fit");

    /* The four together: the sequence a real appender runs. */
    {
        size_t cap = 16, len = 0;

        check(will_fit(cap, len, 5) == 1, "\"alpha\" fits");
        len += 5;
        check(room_left(cap, len) == 10, "and leaves room for 10");
        check(will_fit(cap, len, 11) == 0, "an 11-character append does not fit");
        check(size_needed(len, 11) == 17, "it would need 17 bytes");
        check(size_needed(len, 11) > cap, "which is more than the buffer has");
    }

    return checks_done("s04-r02-room-math-reps");
}
