/* s01-e07-strlen-lookup: answers from the manual pages, graded.
 *
 * Fill in answers.txt from `man`, not from memory and not by experiment. Three of the
 * nine are about behaviour, and for those this program makes the call and compares the
 * outcome with what you say the page promises. The other six are about the
 * documentation itself -- which standard, which header, which feature-test macro --
 * and for those it compares a hash of your answer, so it is not in this file.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* Through functions, so that each answer is a real call at run time. */
static size_t len_of(const char *s)
{
    return strlen(s);
}

static size_t nlen_of(const char *s, size_t maxlen)
{
    return strnlen(s, maxlen);
}

int main(int argc, char **argv)
{
    static const char full[4] = {'A', '1', '2', '3'}; /* no terminator */

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Behaviour: the page says it, the call shows it. */
    answer_int("strlen.empty", (long)len_of(""));
    answer_int("strnlen.zero", (long)nlen_of("abc", 0));
    answer_int("strnlen.full_field", (long)nlen_of(full, sizeof full));

    /* Documentation: compared by hash. */
    answer_hash("strlen.in_iso_c", UINT32_C(0xbeb4692a));
    answer_hash("strnlen.in_iso_c", UINT32_C(0xbf96515a));
    answer_hash("strnlen.macro", UINT32_C(0x8ee8d818));
    answer_hash("strnlen.macro_value", UINT32_C(0x53ba98e3));
    answer_hash("strnlen.header", UINT32_C(0x54d7facc));
    answer_hash("strlen.return_type", UINT32_C(0xdb94b0bf));

    return answers_done("s01-e07-strlen-lookup");
}
