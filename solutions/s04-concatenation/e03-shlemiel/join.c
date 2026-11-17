/* join.c -- s04-e03-shlemiel, fixed: the destination is never rescanned. The length
 * written so far is kept in `need`, which is the same number the function already
 * computes for its return value, so the fix costs no extra state at all. */
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
            if (need + 1 < cap) {
                dst[need] = ' ';
                dst[need + 1] = '\0';
            }
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
