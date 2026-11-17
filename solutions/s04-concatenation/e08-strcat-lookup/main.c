/* s04-e08-strcat-lookup: answers from the manual pages, graded.
 *
 * Fill in answers.txt from `man`, not from memory and not by experiment. Four of the
 * nine are about behaviour, and for those this program makes the call and compares the
 * outcome with what you say the page promises. The other five are about the
 * documentation itself -- a size formula, a return value, a standard -- and for those
 * it compares a hash of your answer.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* Through functions, so that the sizes arrive as parameters and gcc has nothing to
 * say about the calls below. */
static char *cat(char *dst, const char *src)
{
    return strcat(dst, src);
}

static char *ncat(char *dst, const char *src, size_t n)
{
    return strncat(dst, src, n);
}

int main(int argc, char **argv)
{
    char buf[16];

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Behaviour: the page says it, the call shows it. */
    memcpy(buf, "ab", 3);
    answer_str("strcat.returns", cat(buf, "cd") == buf ? "dst" : "other");

    memcpy(buf, "ab", 3);
    ncat(buf, "cdef", 2);
    answer_str("strncat.result", buf);

    /* How many bytes does strncat write in all, for a source longer than n? */
    memset(buf, '#', sizeof buf);
    memcpy(buf, "ab", 3);
    ncat(buf, "cdef", 2);
    answer_int("strncat.bytes_written", (long)(strlen(buf) + 1 - strlen("ab")));

    /* strncat with n larger than the source stops at the source's terminator. */
    memcpy(buf, "ab", 3);
    ncat(buf, "cd", 9);
    answer_str("strncat.short_src", buf);

    /* Documentation: compared by hash. */
    answer_hash("strcat.need", UINT32_C(0xc1b45c90));
    answer_hash("strncat.need", UINT32_C(0x54b9b417));
    answer_hash("strncat.max_bytes", UINT32_C(0x2cfcc4ac));
    answer_hash("strncat.reports_truncation", UINT32_C(0x8f7e89e1));
    answer_hash("strlcat.in_posix_2008", UINT32_C(0xb32f5dd0));

    return answers_done("s04-e08-strcat-lookup");
}
