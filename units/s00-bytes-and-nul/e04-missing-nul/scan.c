/* scan.c -- s00-e04-missing-nul: one of these two functions reads a byte it does not
 * own. Do not change anything until you have answered the questions in answers.txt. */
#include "scan.h"

#include <string.h>

/* Copies exactly n bytes from src to dst. No terminator: a fixed-width field is as
 * wide as it is, and the caller knows the width. */
static void field_copy(char *dst, const char *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
}

size_t entry_id_length(const char *line)
{
    char id[ID_SIZE];

    field_copy(id, line, ID_SIZE);
    return strlen(id);
}

int entry_count(const char *line)
{
    size_t i = 0;

    while (line[i] != '\0' && line[i] != ',')
        i++;
    if (line[i] == '\0' || line[i + 1] < '0' || line[i + 1] > '9')
        return -1;
    return line[i + 1] - '0';
}
