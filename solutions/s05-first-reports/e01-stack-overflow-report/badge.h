/* badge.h -- s05-e01-stack-overflow-report: printing a visitor badge. */
#ifndef BADGE_H
#define BADGE_H

#include <stddef.h>

/* Writes "<name> (visitor)" into out, which has cap bytes. Returns the length it
 * needed, like snprintf: a return value >= cap means the text was truncated. */
size_t badge_line(char *out, size_t cap, const char *name);

/* Prints the badge for `name` to stdout. */
void badge_print(const char *name);

#endif
