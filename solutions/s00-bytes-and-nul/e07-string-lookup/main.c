/* s00-e07-string-lookup: answers from the manual pages, graded.
 *
 * Fill in answers.txt from `man`, not from memory and not by experiment. Two of the
 * nine are about behaviour, and for those this program makes the call and compares the
 * outcome with what you say the page promises. The other seven are about the
 * documentation itself (a header, a type, a word), and for those it compares a hash of
 * your answer, so the answer is not in this file either.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* The length of a three-character string, through a function, so that the comparison
 * below is a real call and not a constant the compiler worked out. */
static size_t length_of(const char *s)
{
    return strlen(s);
}

int main(int argc, char **argv)
{
    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Behaviour: the page says it, the call shows it. */
    answer_str("strlen.counts_terminator", length_of("abc") == 4 ? "yes" : "no");
    answer_int("strlen.empty", (long)length_of(""));

    /* Documentation: compared by hash. */
    answer_hash("strlen.header", UINT32_C(0x6f28215e));
    answer_hash("strlen.return_type", UINT32_C(0xdb94b0bf));
    answer_hash("strlen.param_const", UINT32_C(0xff78636e));
    answer_hash("strlen.unterminated", UINT32_C(0x017d74fc));
    answer_hash("strlen.bounded_name", UINT32_C(0xe7c7824d));
    answer_hash("puts.appends_newline", UINT32_C(0xce32549d));
    answer_hash("puts.error_return", UINT32_C(0x8aa0b081));

    return answers_done("s00-e07-string-lookup");
}
