/* join.c -- s04-e03-shlemiel. This file produces the right answer and is too slow.
 * The defect is in how it works, not in what it computes. */
#include "join.h"

#include <string.h>

size_t join_words(char *dst, size_t cap, const char *const *words, size_t n)
{
    size_t need = 0;

    if (cap > 0)
        dst[0] = '\0';
    for (size_t i = 0; i < n; i++) {
        size_t wlen = strlen(words[i]);

        if (i > 0) {
            if (need + 1 < cap)
                strcat(dst, " ");
            need += 1;
        }
        if (need + wlen < cap)
            strcat(dst, words[i]);
        need += wlen;
    }
    return need;
}
