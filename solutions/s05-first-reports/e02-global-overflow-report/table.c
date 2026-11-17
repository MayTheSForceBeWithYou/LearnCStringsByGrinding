/* table.c -- s05-e02-global-overflow-report. Do not change anything until you have
 * answered the questions in answers.txt. */
#include "table.h"

#include <stdio.h>

static const char months[MONTHS][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

const char *month_name(int m)
{
    return months[m - 1];
}

size_t month_label(char *out, size_t cap, int m, int day)
{
    int n = snprintf(out, cap, "%s %d", month_name(m), day);

    return n < 0 ? 0 : (size_t)n;
}
