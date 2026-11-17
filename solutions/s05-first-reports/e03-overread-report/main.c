/* s05-e03-overread-report: read the report, then answer. */

/* With no arguments this program is only the grader for answers.txt. With the argument
 * `bad` it prints a heap copy of a record's code field, which is how capture.txt was
 * made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "answers.h"
#include "record.h"

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        Record r;
        char *code;

        record_set(&r, "AB1234", 7); /* six characters: the field is full */
        code = record_code_dup(&r);
        if (code == NULL)
            return 3;
        printf("%s x%d\n", code, r.quantity);
        free(code);
        return 0;
    }

    answers_load("answers.txt");

    /* Two facts the program can state by compiling. */
    answer_int("field.width", CODE_WIDTH);
    answer_int("dup.bytes", CODE_WIDTH + 1);

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("error.kind", UINT32_C(0x73cce4a6));
    answer_hash("access.kind", UINT32_C(0xdff6784c));
    answer_hash("access.size", UINT32_C(0xf73d1cd2));
    answer_hash("region.size", UINT32_C(0xb8a1f8eb));
    answer_hash("fault.function", UINT32_C(0x82e8d093));
    answer_hash("fault.line", UINT32_C(0x232c84d6));
    answer_hash("alloc.function", UINT32_C(0x1dc796b4));
    answer_hash("alloc.line", UINT32_C(0xaaa0b8c4));

    return answers_done("s05-e03-overread-report");
}
