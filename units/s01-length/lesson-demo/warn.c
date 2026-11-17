/* warn.c -- lesson-demo for S01: the one thing the compiler can work out for itself
 * about a length, and two it cannot. Never linked or run;
 * `make capture TOOL=warn` records the diagnostics. */
#include <stdio.h>
#include <string.h>

/* The identifier field of a constant record: four characters, no terminator. */
static const char id[4] = {'A', '1', '2', '3'};

/* 1. strlen of an object the compiler can see has no terminator in it. */
size_t id_length(void)
{
    return strlen(id);
}

/* 2. The loop bound of e03, with a length the compiler can also work out. */
size_t count_upper_of_empty(void)
{
    const char *s = "";
    size_t n = 0;

    for (size_t i = 0; i <= strlen(s) - 1; i++) {
        if (s[i] >= 'A' && s[i] <= 'Z')
            n++;
    }
    return n;
}

/* 3. A length stored in a type that cannot hold every length. */
unsigned char name_length(const char *s)
{
    return strlen(s);
}
