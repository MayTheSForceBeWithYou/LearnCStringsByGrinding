/* s04-e03-shlemiel: harness. The defect is in join.c, not here.
 *
 * The output is checked first, then the cost. The budget is measured on this machine:
 * the harness times one linear pass that builds the same string with memcpy, and
 * requires join_words to be within a generous multiple of it. A correct join beats the
 * budget by a wide margin on any machine; a join that rescans the destination on every
 * word does not.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "course.h"
#include "join.h"
#include "strcheck.h"

enum { WORDS = 60000, WORD_LEN = 4 };

static double cpu_since(clock_t t0)
{
    return (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
}

/* The same join, written as one pass with memcpy: the reference for the budget. */
static size_t ref_join(char *dst, size_t cap, const char *const *words, size_t n)
{
    size_t need = 0;

    if (cap > 0)
        dst[0] = '\0';
    for (size_t i = 0; i < n; i++) {
        size_t wlen = strlen(words[i]);

        if (i > 0) {
            if (need + 1 < cap)
                dst[need] = ' ';
            need += 1;
        }
        if (need + wlen < cap) {
            memcpy(dst + need, words[i], wlen);
            dst[need + wlen] = '\0';
        }
        need += wlen;
    }
    return need;
}

int main(void)
{
    static const char *const three[] = {"alpha", "beta", "gamma"};
    char small[32];

    /* Correctness first: the small cases say what the function is for. */
    check(join_words(small, sizeof small, three, 3) == 16, "three words and two spaces is 16 characters");
    check_str(small, "alpha beta gamma", "the three words, space-separated");
    check(join_words(small, sizeof small, three, 1) == 5, "one word has no separator");
    check_str(small, "alpha", "one word");
    check(join_words(small, sizeof small, three, 0) == 0, "no words is the empty string");
    check_str(small, "", "no words");
    check(join_words(small, 8, three, 3) == 16, "a truncated join still reports the full length");
    check_str(small, "alpha ", "and stores the pieces that fitted");
    check(join_words(small, 1, three, 3) == 16, "cap 1 stores only a terminator");
    check_str(small, "", "cap 1");

    /* Now the cost. Build 20000 short words and join them twice. */
    {
        char **words = malloc(WORDS * sizeof *words);
        char *pool = malloc((size_t)WORDS * (WORD_LEN + 1));
        size_t cap = (size_t)WORDS * (WORD_LEN + 1) + 1;
        char *big = malloc(cap);
        char *ref = malloc(cap);
        double t_ref, t_got;
        size_t n_ref, n_got;

        if (words == NULL || pool == NULL || big == NULL || ref == NULL) {
            fprintf(stderr, "s04-e03-shlemiel: out of memory\n");
            return EXIT_OOM;
        }
        for (size_t i = 0; i < WORDS; i++) {
            char *w = pool + i * (WORD_LEN + 1);

            for (int k = 0; k < WORD_LEN; k++)
                w[k] = (char)('a' + (int)((i + (size_t)k) % 26));
            w[WORD_LEN] = '\0';
            words[i] = w;
        }

        {   /* The reference pass, timed as the best of three. */
            t_ref = 1e9;
            for (int k = 0; k < 3; k++) {
                clock_t t0 = clock();
                n_ref = ref_join(ref, cap, (const char *const *)words, WORDS);
                double d = cpu_since(t0);

                if (d < t_ref)
                    t_ref = d;
            }
        }
        {
            clock_t t0 = clock();
            n_got = join_words(big, cap, (const char *const *)words, WORDS);
            t_got = cpu_since(t0);
        }
        check(n_got == n_ref, "the long join reports the same length as the reference");
        check(strcmp(big, ref) == 0, "the long join produces the same string as the reference");
        check(t_got < 50.0 * t_ref + 0.005,
              "join_words costs at most 50 linear passes (no rescanning the destination)");
        if (t_got >= 50.0 * t_ref + 0.005)
            printf("       reference %.4f s, join_words %.4f s, budget %.4f s\n",
                   t_ref, t_got, 50.0 * t_ref + 0.005);
        free(ref);
        free(big);
        free(pool);
        free(words);
    }

    return checks_done("s04-e03-shlemiel");
}
