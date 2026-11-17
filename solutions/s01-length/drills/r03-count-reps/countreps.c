/* countreps.c -- s01-r03-count-reps: your four functions. */
#include "countreps.h"

/* ---- Your definitions of the countreps.h functions go below this line. ---- */

size_t count_byte(const char *s, char c)
{
    size_t n = 0;

    if (c == '\0')
        return 0;
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == c)
            n++;
    }
    return n;
}

static int is_sep(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

size_t count_words(const char *s)
{
    size_t n = 0;
    int in_word = 0;

    for (size_t i = 0; s[i] != '\0'; i++) {
        if (is_sep(s[i])) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            n++;
        }
    }
    return n;
}

size_t count_lines(const char *s)
{
    size_t n = 0;

    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == '\n' || s[i + 1] == '\0')
            n++;
    }
    return n;
}

size_t longest_run(const char *s)
{
    size_t best = 0, run = 0;

    for (size_t i = 0; s[i] != '\0'; i++) {
        run = (i > 0 && s[i] == s[i - 1]) ? run + 1 : 1;
        if (run > best)
            best = run;
    }
    return best;
}
