/* s04-e06-room-predict: twelve appends into an 8-byte buffer.
 *
 * Fill in answers.txt BEFORE you run this. Each call is made for real on a buffer that
 * holds the bytes the table says, and the program compares three things with your
 * prediction: the buffer afterwards, the call's return value, and the room left for
 * characters afterwards.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

enum { BUF = 8 };
enum fn { STRCAT, STRNCAT, SNPRINTF, STRLCAT_LIKE };

/* The calls. `before` is the exact contents of the 8-byte buffer, `src` the string to
 * append, `n` the third argument where there is one. */
static const struct {
    const char *key;
    enum fn fn;
    const char *before;
    const char *src;
    size_t n;
} calls[] = {
    {"call01", STRCAT, "ab\0#####", "cd", 0},
    {"call02", STRCAT, "abcdef\0#", "g", 0},
    {"call03", STRNCAT, "ab\0#####", "cdefgh", 3},
    {"call04", STRNCAT, "ab\0#####", "cd", 5},
    {"call05", STRNCAT, "ab\0#####", "cdefgh", 5},
    {"call06", STRNCAT, "abcdef\0#", "gh", 1},
    {"call07", SNPRINTF, "ab\0#####", "cd", 0},
    {"call08", SNPRINTF, "ab\0#####", "cdefghij", 0},
    {"call09", SNPRINTF, "abcdefg\0", "h", 0},
    {"call10", STRLCAT_LIKE, "ab\0#####", "cd", 0},
    {"call11", STRLCAT_LIKE, "ab\0#####", "cdefghij", 0},
    {"call12", STRLCAT_LIKE, "abcdefg\0", "h", 0},
};

/* The BSD append's contract, written out because the course cannot call that function:
 * append as much as fits, always terminate, and return the length it tried to create. */
static size_t strlcat_like(char *dst, const char *src, size_t size)
{
    size_t dlen = strnlen(dst, size);
    size_t slen = strlen(src);

    if (dlen == size)
        return size + slen;
    for (size_t i = 0; i < slen && dlen + i + 1 < size; i++)
        dst[dlen + i] = src[i];
    {
        size_t r = size - dlen - 1;
        size_t keep = slen < r ? slen : r;

        dst[dlen + keep] = '\0';
    }
    return dlen + slen;
}

/* Through functions, so that gcc cannot see the sizes and comment on them. */
static char *call_strncat(char *dst, const char *src, size_t n)
{
    return strncat(dst, src, n);
}

static int call_snprintf(char *dst, size_t cap, const char *a, const char *b)
{
    return snprintf(dst, cap, "%s%s", a, b);
}

int main(int argc, char **argv)
{
    char buf[BUF], key[32];

    answers_load(argc > 1 ? argv[1] : "answers.txt");
    for (size_t i = 0; i < sizeof calls / sizeof calls[0]; i++) {
        long ret = 0;
        char before[BUF];

        memcpy(before, calls[i].before, BUF);
        memcpy(buf, before, BUF);
        switch (calls[i].fn) {
        case STRCAT: /* .ret is the offset of the returned pointer from buf */
            ret = strcat(buf, calls[i].src) - buf;
            break;
        case STRNCAT:
            ret = call_strncat(buf, calls[i].src, calls[i].n) - buf;
            break;
        case SNPRINTF: /* a fresh format of dst's old text and src, like an append */
            ret = call_snprintf(buf, BUF, before, calls[i].src);
            break;
        case STRLCAT_LIKE:
            ret = (long)strlcat_like(buf, calls[i].src, BUF);
            break;
        }
        snprintf(key, sizeof key, "%s.buf", calls[i].key);
        answer_bytes(key, buf, BUF);
        snprintf(key, sizeof key, "%s.ret", calls[i].key);
        answer_int(key, ret);
        snprintf(key, sizeof key, "%s.room", calls[i].key);
        answer_int(key, (long)(BUF - strnlen(buf, BUF) - 1));
    }
    return answers_done("s04-e06-room-predict");
}
