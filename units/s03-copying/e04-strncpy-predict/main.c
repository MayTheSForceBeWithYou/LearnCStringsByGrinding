/* s03-e04-strncpy-predict: what is in the buffer after each call, and what came back?
 *
 * Fill in answers.txt BEFORE you run this. The program makes each call for real, on an
 * 8-byte buffer that holds ######## beforehand, and compares the outcome with your
 * prediction. It tells you which lines are wrong, never what the right value is.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

enum { BUF = 8 };
enum fn { STRNCPY, STPNCPY, SNPRINTF };

/* The ten calls. Read each row as:  fn(buf, <src>, <n>)  or  snprintf(buf, <n>, "%s", <src>). */
static const struct {
    const char *key;
    enum fn fn;
    const char *src;
    size_t n;
} calls[] = {
    {"call01", STRNCPY, "hello", 8},        /* strncpy(buf, "hello", 8)                  */
    {"call02", STRNCPY, "hello", 5},        /* strncpy(buf, "hello", 5)                  */
    {"call03", STRNCPY, "hello", 3},        /* strncpy(buf, "hello", 3)                  */
    {"call04", STRNCPY, "", 4},             /* strncpy(buf, "", 4)                       */
    {"call05", STRNCPY, "abcdefgh", 8},     /* strncpy(buf, "abcdefgh", 8)               */
    {"call06", STPNCPY, "hey", 8},          /* stpncpy(buf, "hey", 8)                    */
    {"call07", STPNCPY, "hello", 5},        /* stpncpy(buf, "hello", 5)                  */
    {"call08", SNPRINTF, "hello", 8},       /* snprintf(buf, 8, "%s", "hello")           */
    {"call09", SNPRINTF, "abcdefghij", 8},  /* snprintf(buf, 8, "%s", "abcdefghij")      */
    {"call10", SNPRINTF, "hello", 4},       /* snprintf(buf, 4, "%s", "hello")           */
};

int main(int argc, char **argv)
{
    char buf[BUF], key[32];

    answers_load(argc > 1 ? argv[1] : "answers.txt");
    for (size_t i = 0; i < sizeof calls / sizeof calls[0]; i++) {
        long ret = 0;
        memset(buf, '#', BUF);
        switch (calls[i].fn) {
        case STRNCPY: /* .ret is the offset of the returned pointer from buf */
            ret = strncpy(buf, calls[i].src, calls[i].n) - buf;
            break;
        case STPNCPY:
            ret = stpncpy(buf, calls[i].src, calls[i].n) - buf;
            break;
        case SNPRINTF: /* .ret is the int that snprintf returned */
            ret = snprintf(buf, calls[i].n, "%s", calls[i].src);
            break;
        }
        snprintf(key, sizeof key, "%s.buf", calls[i].key);
        answer_bytes(key, buf, BUF);
        snprintf(key, sizeof key, "%s.ret", calls[i].key);
        answer_int(key, ret);
    }
    return answers_done("s03-e04-strncpy-predict");
}
