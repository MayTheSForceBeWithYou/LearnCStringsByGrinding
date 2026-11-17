/* s01-r03-count-reps: harness. Do not edit; write your code in countreps.c. */
#include <stdio.h>
#include <string.h>

#include "countreps.h"
#include "course.h"
#include "oracle.h"
#include "strcheck.h"

static void one(const char *s, char c, size_t bytes, size_t words, size_t lines, size_t run)
{
    char *only = guard_copy(s, strlen(s) + 1);

    check(count_byte(only, c) == bytes, "count_byte");
    check(count_words(only) == words, "count_words");
    check(count_lines(only) == lines, "count_lines");
    check(longest_run(only) == run, "longest_run");
    check_guard(only, "nothing was written");
    guard_free(only);
}

/* The same four counts, computed here by the most obvious loop, over generated input. */
static void oracle(void)
{
    enum { MAXLEN = 30, CASES = 2000 };
    char src[MAXLEN + 1];
    int ok = 1;

    orc_seed(23);
    for (int i = 0; i < CASES && ok; i++) {
        size_t len = orc_string(src, MAXLEN, ORC_ALPHA_TEXT);
        char *only = guard_copy(src, len + 1);
        size_t w_bytes = 0, w_words = 0, w_lines = 0, w_run = 0, run = 0;

        for (size_t j = 0; j < len; j++) {
            int sep = only[j] == ' ' || only[j] == '\t' || only[j] == '\n';

            if (only[j] == 'a')
                w_bytes++;
            if (!sep && (j == 0 || only[j - 1] == ' ' || only[j - 1] == '\t' || only[j - 1] == '\n'))
                w_words++;
            if (only[j] == '\n')
                w_lines++;
            else if (j + 1 == len)
                w_lines++;
            run = (j > 0 && only[j] == only[j - 1]) ? run + 1 : 1;
            if (run > w_run)
                w_run = run;
        }
        if (count_byte(only, 'a') != w_bytes || count_words(only) != w_words ||
            count_lines(only) != w_lines || longest_run(only) != w_run) {
            ok = 0;
            printf("       seed 23, case %u, s: %s\n", (unsigned)orc_case(), bytes_show(only, len + 1));
            printf("       count_byte %zu (want %zu), count_words %zu (want %zu)\n",
                   count_byte(only, 'a'), w_bytes, count_words(only), w_words);
            printf("       count_lines %zu (want %zu), longest_run %zu (want %zu)\n",
                   count_lines(only), w_lines, longest_run(only), w_run);
        }
        guard_free(only);
    }
    check(ok, "all four agree with a byte-by-byte count on 2000 generated strings");
}

int main(void)
{
    /*        s                      c     byte words lines  run */
    one("",                         'a',    0,    0,    0,    0);
    one("a",                        'a',    1,    1,    1,    1);
    one("banana",                   'a',    3,    1,    1,    1);
    one("aabbba",                   'b',    3,    1,    1,    3);
    one("  a  bb  ",                'b',    2,    2,    1,    2);
    one("one two three",            'e',    3,    3,    1,    2);
    one("a\nb",                     'a',    1,    2,    2,    1);
    one("a\n",                      'a',    1,    1,    1,    1);
    one("\n",                       '\n',   1,    0,    1,    1);
    one("\n\n\n",                   '\n',   3,    0,    3,    3);
    one("x\ty z\nw",                'x',    1,    4,    2,    1);
    one("caf\xC3\xA9",              '\xA9', 1,    1,    1,    1);
    one("\xFF\xFF\xFF",             '\xFF', 3,    1,    1,    3);

    /* The terminator is never counted. */
    {
        char *only = guard_copy("ab", 3);
        check(count_byte(only, '\0') == 0, "count_byte does not count the terminator");
        guard_free(only);
    }

    oracle();
    return checks_done("s01-r03-count-reps");
}
