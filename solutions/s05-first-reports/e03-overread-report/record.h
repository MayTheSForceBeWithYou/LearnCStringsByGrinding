/* record.h -- s05-e03-overread-report: a record with a fixed-width code field. */
#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

#define CODE_WIDTH 6 /* the code field is exactly six characters, with no terminator */

typedef struct {
    char code[CODE_WIDTH];
    int quantity;
} Record;

/* Fills a record from a six-character code and a quantity. */
void record_set(Record *r, const char *code, int quantity);

/* A heap copy of the record's code field, which the caller frees.
 * Returns NULL if allocation fails. */
char *record_code_dup(const Record *r);

#endif
