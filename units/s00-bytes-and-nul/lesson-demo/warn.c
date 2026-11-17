/* warn.c -- lesson-demo for S00: what the compiler says when it CAN see the mistake.
 * Never linked or run; `make capture TOOL=warn CAPTURE=capture-warn.txt` records the
 * diagnostics. Each item is one defect the lesson describes. */
#include <stdio.h>
#include <string.h>

/* 1. A four-byte literal into a three-byte array: the terminator is dropped. */
char code[3] = "abc";

/* 2. sizeof on a pointer, used as if it were the size of what it points at. */
void clear(char *p)
{
    memset(p, 0, sizeof p);
}

/* 3. An offset into a literal, written as an addition. */
const char *tail(void)
{
    return "hello" + 2;
}

/* 4. strlen of an array the compiler can see has no terminator. */
size_t code_length(void)
{
    return strlen(code);
}
