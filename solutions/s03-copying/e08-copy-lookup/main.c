/* s03-e08-copy-lookup: answers from the manual pages, graded.
 *
 * Fill in answers.txt from `man`, not from memory and not by experiment. Where a fact
 * is about behaviour, this program makes the call and compares the outcome with what
 * you say the page promises; where it is about the documentation itself (a header, a
 * standard), it compares a hash of your answer, so the answer is not in this file.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* "dst", "end" or "other": what a copy function's return value points at. */
static const char *where(const char *ret, const char *dst, size_t len)
{
    if (ret == dst + len && len > 0)
        return "end";
    return ret == dst ? "dst" : "other";
}

/* The calls go through these two so that the compiler, which also knows what strncpy
 * does with a full-length source, has nothing to say about them. */
static char *call_strncpy(char *dst, const char *src, size_t n)
{
    return strncpy(dst, src, n);
}

static char *call_stpncpy(char *dst, const char *src, size_t n)
{
    return stpncpy(dst, src, n);
}

int main(int argc, char **argv)
{
    char buf[8];

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Behaviour: the page says it, the call shows it. */
    answer_str("strcpy.returns", where(strcpy(buf, "abc"), buf, 3));
    answer_str("stpcpy.returns", where(stpcpy(buf, "abc"), buf, 3));

    memset(buf, '#', sizeof buf);
    call_strncpy(buf, "ab", 6);
    answer_str("strncpy.pads", buf[2] == '\0' && buf[5] == '\0' && buf[6] == '#' ? "yes" : "no");

    memset(buf, '#', sizeof buf);
    call_strncpy(buf, "abcdef", 6);
    answer_str("strncpy.terminates_full", memchr(buf, '\0', sizeof buf) != NULL ? "yes" : "no");

    memset(buf, '#', sizeof buf);
    answer_int("stpncpy.full_offset", call_stpncpy(buf, "abcdef", 6) - buf);

    /* Documentation: compared by hash. */
    answer_hash("strcpy.header", UINT32_C(0xff9d19b7));
    answer_hash("strcpy.overlap_allowed", UINT32_C(0xf148f358));
    answer_hash("stpcpy.iso_c17", UINT32_C(0x50c7458f));
    answer_hash("strncpy.reports_truncation", UINT32_C(0x45bbd477));

    return answers_done("s03-e08-copy-lookup");
}
