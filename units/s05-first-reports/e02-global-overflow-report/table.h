/* table.h -- s05-e02-global-overflow-report: a table of month abbreviations. */
#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

#define MONTHS 12

/* The three-letter abbreviation for month m, which must be 1 to 12. Returns a pointer
 * to storage that belongs to this module: read it, do not write it, do not free it. */
const char *month_name(int m);

/* Writes "<abbrev> <day>" into out, which has cap bytes. Returns the length needed. */
size_t month_label(char *out, size_t cap, int m, int day);

#endif
