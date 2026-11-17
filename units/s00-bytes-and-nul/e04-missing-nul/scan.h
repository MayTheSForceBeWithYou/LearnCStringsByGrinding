/* scan.h -- s00-e04-missing-nul: reading fixed-width fields out of a record line. */
#ifndef SCAN_H
#define SCAN_H

#include <stddef.h>

#define ID_SIZE 4 /* the id field of a record line is exactly four characters */

/* The number of characters in the id field at the start of the record line.
 *
 * A record line looks like "A123,7": a four-character id, a comma, then a count.
 * `line` is borrowed and must have at least ID_SIZE readable bytes.
 * Returns the length of the id. */
size_t entry_id_length(const char *line);

/* The count after the comma, or -1 if the line is not a record line. */
int entry_count(const char *line);

#endif
