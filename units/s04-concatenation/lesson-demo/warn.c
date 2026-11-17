/* warn.c -- lesson-demo for S04: the appends this unit is about, written where the
 * compiler can see the sizes. Never linked or run; `make capture TOOL=warn` records
 * the diagnostics. */
#include <stdio.h>
#include <string.h>

/* 1. strncat given the destination's size instead of the room left in it. */
void add_tag(char *dst, const char *src)
{
    char buf[8] = "ab";

    strncat(buf, src, sizeof buf);
    strcpy(dst, buf);
}

/* 2. strcat into a buffer the compiler can see is too small. */
void greet(void)
{
    char small[8] = "hello";

    strcat(small, ", world");
    puts(small);
}

/* 3. The same append done with snprintf, which truncates instead. */
int label(const char *text)
{
    char buf[8];

    return snprintf(buf, sizeof buf, "warn: %s", text);
}
