/* s01-e04-length-predict: sixteen measurements of five objects.
 *
 * Fill in answers.txt BEFORE you run this. The program evaluates each expression for
 * real and compares the result with your prediction. One expression it does not
 * evaluate, because the standard does not define what it would do; that answer is the
 * word `undefined`, graded by a hash.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

int main(int argc, char **argv)
{
    char a[] = "hello";                  /* 6 bytes                             */
    char e[] = "ab\0cd";                 /* 6 bytes, with a NUL in the middle   */
    char f[8] = "hi";                    /* 8 bytes, zero-filled past the text  */
    const char *p = &a[2];               /* a pointer into the middle of a      */
    char full[4] = {'A', '1', '2', '3'}; /* 4 characters, no terminator         */

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    answer_int("a.strlen", (long)strlen(a));
    answer_int("a.sizeof", (long)sizeof a);
    answer_int("a.strnlen0", (long)strnlen(a, 0));
    answer_int("a.strnlen3", (long)strnlen(a, 3));
    answer_int("a.strnlen6", (long)strnlen(a, 6));

    answer_int("e.strlen", (long)strlen(e));
    answer_int("e.sizeof", (long)sizeof e);
    answer_int("e.tail_strlen", (long)strlen(&e[3]));

    answer_int("f.strlen", (long)strlen(f));
    answer_int("f.sizeof", (long)sizeof f);
    answer_int("f.strnlen8", (long)strnlen(f, 8));

    answer_int("p.strlen", (long)strlen(p));
    answer_int("p.sizeof", (long)sizeof p);

    answer_int("full.sizeof", (long)sizeof full);
    answer_int("full.strnlen4", (long)strnlen(full, 4));
    answer_hash("full.strlen", UINT32_C(0x9eb0be15));

    return answers_done("s01-e04-length-predict");
}
