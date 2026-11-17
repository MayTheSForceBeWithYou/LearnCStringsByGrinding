/* s02-e08-mem-lookup: answers from the manual pages, graded.
 *
 * Fill in answers.txt from `man`, not from memory and not by experiment. Four of the
 * nine are about behaviour, and for those this program makes the call and compares the
 * outcome with what you say the page promises. The other five are about the
 * documentation itself -- a qualifier in a prototype, a sentence about overlap, the
 * type bytes are compared as -- and for those it compares a hash of your answer.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* Through functions, so that each answer below is a real call at run time. */
static void *cpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static void *mov(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}

int main(int argc, char **argv)
{
    char dst[9] = "........";
    char src[9] = "abcdefgh";

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Behaviour: the page says it, the call shows it. */
    answer_str("memcpy.returns", cpy(dst, src, 4) == dst ? "dst" : "other");
    answer_str("memmove.returns", mov(dst, src, 4) == dst ? "dst" : "other");
    answer_str("memchr.not_found", memchr(src, 'z', 8) == NULL ? "null" : "other");
    answer_int("memcmp.zero_n", memcmp(src, dst, 0));

    /* Documentation: compared by hash. */
    answer_hash("memcpy.restrict", UINT32_C(0x6adeb4d4));
    answer_hash("memmove.restrict", UINT32_C(0xbe91ee9d));
    answer_hash("memcpy.overlap_ok", UINT32_C(0xd0cdc478));
    answer_hash("memcmp.compares_as", UINT32_C(0x1594f105));
    answer_hash("memset.converts_to", UINT32_C(0x65991d94));

    return answers_done("s02-e08-mem-lookup");
}
