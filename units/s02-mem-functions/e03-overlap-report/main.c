/* s02-e03-overlap-report: read the report, then answer. Do not run anything first.
 *
 * With no arguments this program is only the grader for answers.txt. With the argument
 * `bad` it performs the defective edit, which is how capture.txt was made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "text.h"

int main(int argc, char **argv)
{
    char buf[40] = "abcd" "efghijklmnopqrstuvwxyz0123";

    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        size_t len = strlen(buf);

        printf("before: %s\n", buf);
        len = text_delete(buf, len, 0, 4); /* drop the first four characters */
        printf("after:  %s (length %zu)\n", buf, len);
        return 0;
    }

    answers_load("answers.txt");

    /* One fact the program can state by calling the function that is correct. */
    answer_int("move.bytes", (long)text_move_count(strlen(buf), 0, 4));

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("error.kind", UINT32_C(0x41eaf1c1));
    answer_hash("range.size", UINT32_C(0xa490a383));
    answer_hash("gap.bytes", UINT32_C(0x1c48886f));
    answer_hash("overlap.bytes", UINT32_C(0xd49a1a83));
    answer_hash("fault.file", UINT32_C(0xd21468e8));
    answer_hash("fault.line", UINT32_C(0x1f2a3ff3));
    answer_hash("fix.function", UINT32_C(0x88021247));
    answer_hash("forward_loop_ok", UINT32_C(0x6dede0d2));

    return answers_done("s02-e03-overlap-report");
}
