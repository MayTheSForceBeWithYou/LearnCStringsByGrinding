/* warn.c -- lesson-demo for S05: four defects written where the compiler can see
 * them, so that the lesson can quote what it says. Never linked or run;
 * `make capture TOOL=warn` records the diagnostics. */
#include <stdio.h>
#include <string.h>

/* 1. A copy the compiler can size on both sides. */
void greet(void)
{
    char small[8];

    strcpy(small, "a much longer greeting");
    puts(small);
}

/* 2. A format specifier that does not match its argument. */
int count(size_t n)
{
    return printf("%d items\n", n);
}

/* 3. sizeof on a pointer, as a byte count. */
void clear(char *p)
{
    memset(p, 0, sizeof p);
}

/* 4. A formatted write that is known to be cut short. */
int label(void)
{
    char buf[8];

    return snprintf(buf, sizeof buf, "%s", "abcdefghij");
}
