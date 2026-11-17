/* warn.c -- lesson-demo for S03: what the compiler says when it CAN see a bad copy.
 * Never linked or run; `make capture TOOL=warn CAPTURE=capture-warn.txt` records the
 * diagnostics. Each function is one defect the lesson describes. */
#include <stdio.h>
#include <string.h>

struct member {
    int id;
    char name[12];
};

/* 1. The bound equals the size of the destination: no room is left for a terminator. */
void set_name(struct member *m, const char *name)
{
    strncpy(m->name, name, sizeof m->name);
}

/* 2. The source is known to be exactly as long as the bound: the terminator is dropped. */
void set_code(char code[6])
{
    strncpy(code, "ABCDEF", 6);
}

/* 3. The source is known to be longer than the destination. */
size_t greeting_length(void)
{
    char small[12];
    strcpy(small, "a name that is too long");
    return strlen(small);
}

/* 4. The formatted output is known to be longer than the buffer. */
int set_label(char label[8])
{
    return snprintf(label, 8, "%s", "abcdefghij");
}
