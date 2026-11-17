/* s01-e05-unterminated-report: read the report, then answer. Do not run anything first.
 *
 * With no arguments this program is only the grader for answers.txt. With the argument
 * `bad` it calls the defective function and then measures what came back, which is how
 * capture.txt was made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "answers.h"
#include "dup.h"

/* The reader: it is handed a string and measures it. It is not the defect. */
static size_t report_width(const char *name)
{
    return strlen(name);
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        static const char field[4] = {'A', '1', '2', '3'};
        char *name = field_dup(field, sizeof field);

        if (name == NULL)
            return 3;
        printf("width: %zu\n", report_width(name));
        free(name);
        return 0;
    }

    answers_load("answers.txt");

    /* One fact the program can state by calling the function that is correct. */
    {
        static const char field[4] = {'A', '1', '2', '3'};
        answer_int("malloc.bytes", (long)field_dup_size(field, sizeof field));
    }

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("access.kind", UINT32_C(0xdff6784c));
    answer_hash("access.size", UINT32_C(0xf53d19ac));
    answer_hash("region.size", UINT32_C(0xbaa1fc11));
    answer_hash("fault.function", UINT32_C(0xb79e0617));
    answer_hash("alloc.file", UINT32_C(0xa587d9b5));
    answer_hash("alloc.line", UINT32_C(0x17991596));
    answer_hash("defect.function", UINT32_C(0x9a2a4721));
    answer_hash("size.function", UINT32_C(0x804817db));

    return answers_done("s01-e05-unterminated-report");
}
