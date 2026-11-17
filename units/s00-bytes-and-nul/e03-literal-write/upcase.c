/* upcase.c -- s00-e03-literal-write. This file is correct; do not change it.
 *
 * The letter test is written out by hand because <ctype.h> is S07, where you will
 * learn why tolower(c) with a plain char argument is a defect of its own. */
#include "upcase.h"

#include <stddef.h>

void upcase(char *s)
{
    for (size_t i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'a' && c <= 'z')
            s[i] = (char)(c - 'a' + 'A');
    }
}
