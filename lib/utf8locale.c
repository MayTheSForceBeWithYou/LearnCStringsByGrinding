/* utf8locale.c -- see utf8locale.h. Plain C17. */
#include "utf8locale.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

const char *course_utf8_locale(void)
{
    static const char *const names[] = {"C.UTF-8", "en_US.UTF-8"};
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
        if (setlocale(LC_ALL, names[i]) != NULL)
            return names[i];
    }
    if (setlocale(LC_CTYPE, "UTF-8") != NULL)
        return "UTF-8";
    fprintf(stderr,
            "course: no UTF-8 locale found (tried C.UTF-8, en_US.UTF-8, UTF-8).\n"
            "course: on Arch Linux: uncomment en_US.UTF-8 in /etc/locale.gen, then run\n"
            "course:     sudo locale-gen\n"
            "course: `locale -a` lists the locales this machine has.\n");
    exit(5);
}
