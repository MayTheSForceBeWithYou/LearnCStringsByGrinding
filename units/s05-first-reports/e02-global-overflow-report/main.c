/* s05-e02-global-overflow-report: read the report, then answer. */

/* With no arguments this program is only the grader for answers.txt. With the argument
 * `bad` it asks for a month the table does not have, which is how capture.txt was made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "table.h"

int main(int argc, char **argv)
{
    char line[32];

    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        int month = 13; /* a month the table does not have */

        month_label(line, sizeof line, month, 1);
        puts(line);
        return 0;
    }

    /* The table works for the months it has. */
    month_label(line, sizeof line, 1, 1);
    month_label(line, sizeof line, 12, 31);

    answers_load("answers.txt");

    /* Two facts the program can state by compiling. */
    answer_int("table.rows", MONTHS);
    answer_int("row.size", (long)sizeof "Jan");

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("error.kind", UINT32_C(0x1203c65b));
    answer_hash("access.kind", UINT32_C(0xdff6784c));
    answer_hash("variable.name", UINT32_C(0xb96ac7f2));
    answer_hash("variable.file", UINT32_C(0x95f59335));
    answer_hash("variable.line", UINT32_C(0xd125620f));
    answer_hash("variable.size", UINT32_C(0x9049fba3));
    answer_hash("fault.function", UINT32_C(0x168b7e11));
    answer_hash("fault.line", UINT32_C(0x2c2a546a));

    return answers_done("s05-e02-global-overflow-report");
}
