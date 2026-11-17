/* greet.c -- s05-e06-fortify-abort. Do not change anything until you have answered the
 * questions in answers.txt. */
#include "greet.h"

#include <stdio.h>
#include <string.h>

void greet_print(const char *name)
{
    char out[GREET_SIZE];

    strcpy(out, "Hello, ");
    strcat(out, name);
    strcat(out, "!");
    puts(out);
}
