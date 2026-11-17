/* s01-e06-narrow-length: harness. The defect is in name.c, not here. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "name.h"
#include "strcheck.h"

/* A string of n copies of 'x', for the long names below. */
static const char *xs(size_t n)
{
    static char buf[NAME_CAP + 64];

    memset(buf, 'x', n);
    buf[n] = '\0';
    return buf;
}

/* A name that fits: stored, measurable, and readable back byte for byte. */
static void ok_case(Name *n, const char *s)
{
    char out[NAME_CAP];

    check(name_set(n, s) == 0, "name_set accepts a name of NAME_MAX characters or fewer");
    check(name_len(n) == strlen(s), "name_len is the stored name's true length");
    check(name_get(n, out, sizeof out) == strlen(s), "name_get returns the name's length");
    check(strcmp(out, s) == 0, "name_get copies the whole name back");
}

/* A name that cannot be stored: -1, and the record is exactly as it was. */
static void reject_case(Name *n, const char *s)
{
    Name before = *n;

    check(name_set(n, s) == -1, "name_set rejects a name longer than NAME_MAX");
    check(memcmp(&before, n, sizeof before) == 0, "a rejected name changes nothing in the record");
}

int main(void)
{
    Name n;

    memset(&n, 0, sizeof n);
    ok_case(&n, "");
    ok_case(&n, "ada");
    ok_case(&n, xs(100));
    ok_case(&n, xs(254));
    ok_case(&n, xs(255)); /* the longest name the one-byte length field can describe */

    reject_case(&n, xs(256)); /* one too many */
    reject_case(&n, xs(300));
    reject_case(&n, xs(511));

    /* After the rejections the record still holds the last name that was accepted. */
    check(name_len(&n) == 255, "a rejected name leaves the stored length alone");
    check(n.buf[254] == 'x' && n.buf[255] == '\0', "a rejected name leaves the stored name alone");

    ok_case(&n, "grace");

    return checks_done("s01-e06-narrow-length");
}
