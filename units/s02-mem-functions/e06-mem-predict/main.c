/* s02-e06-mem-predict: what do the five mem functions do with these arguments?
 *
 * Fill in answers.txt BEFORE you run this. The program makes each call for real and
 * compares the outcome with your prediction. It tells you which lines are wrong, never
 * what the right value is.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "bytes.h"
#include "strcheck.h"

enum { BUF = 8 };

/* The eight pairs for memcmp. Each pair is compared over `n` bytes. */
static const struct {
    const char *key;
    const char *a;
    const char *b;
    size_t n;
} pairs[] = {
    {"cmp01", "abc", "abc", 3},
    {"cmp02", "abc", "abd", 3},
    {"cmp03", "abd", "abc", 3},
    {"cmp04", "abc", "abd", 2},
    {"cmp05", "a", "a", 0},
    {"cmp06", "\x7F", "\x80", 1},
    {"cmp07", "\xFF", "\x01", 1},
    {"cmp08", "a\0b", "a\0c", 3},
};

/* Through a function, so that the sizes below arrive as parameters. */
static void fill(char *p, int c, size_t n)
{
    memset(p, c, n);
}

static void reset(char *buf)
{
    memset(buf, '#', BUF);
}

int main(int argc, char **argv)
{
    char buf[BUF], key[32];

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    for (size_t i = 0; i < sizeof pairs / sizeof pairs[0]; i++) {
        snprintf(key, sizeof key, "%s.sign", pairs[i].key);
        answer_int(key, sign_of(memcmp(pairs[i].a, pairs[i].b, pairs[i].n)));
    }

    /* memchr: an offset, or -1 for NULL. */
    {
        static const char hay[6] = {'a', 'b', '\0', 'c', 'b', 'd'};
        const char *p;

        p = memchr(hay, 'b', 6);
        answer_int("chr01.at", p == NULL ? -1 : (long)(p - hay));
        p = memchr(hay, 'b', 1);
        answer_int("chr02.at", p == NULL ? -1 : (long)(p - hay));
        p = memchr(hay, 'b', 0);
        answer_int("chr03.at", p == NULL ? -1 : (long)(p - hay));
        p = memchr(hay, '\0', 6);
        answer_int("chr04.at", p == NULL ? -1 : (long)(p - hay));
        p = memchr(hay, 'd', 6);
        answer_int("chr05.at", p == NULL ? -1 : (long)(p - hay));
        p = memchr(hay, 'z', 6);
        answer_int("chr06.at", p == NULL ? -1 : (long)(p - hay));
    }

    /* memset: the byte that is actually stored. */
    reset(buf);
    fill(buf, 0x141, 4);
    answer_bytes("set01.buf", buf, BUF);
    reset(buf);
    fill(buf, -1, 2);
    answer_bytes("set02.buf", buf, BUF);
    reset(buf);
    fill(buf, 'x', 0);
    answer_bytes("set03.buf", buf, BUF);

    /* memmove within one buffer: overlapping, both directions. */
    memcpy(buf, "abcdefgh", BUF);
    memmove(buf + 2, buf, 6);
    answer_bytes("mov01.buf", buf, BUF);

    memcpy(buf, "abcdefgh", BUF);
    memmove(buf, buf + 2, 6);
    answer_bytes("mov02.buf", buf, BUF);

    memcpy(buf, "abcdefgh", BUF);
    memmove(buf + 1, buf, 7);
    answer_bytes("mov03.buf", buf, BUF);

    memcpy(buf, "abcdefgh", BUF);
    memmove(buf, buf, BUF);
    answer_bytes("mov04.buf", buf, BUF);

    return answers_done("s02-e06-mem-predict");
}
