/* s00-e04-missing-nul: read the report, then answer. Do not run anything first.
 *
 * With no arguments this program is only the grader for answers.txt. With the
 * argument `bad` it calls the defective function, which is how capture.txt was made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "scan.h"

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        const char *line = "A123,7";
        printf("id length: %zu\n", entry_id_length(line));
        printf("count: %d\n", entry_count(line));
        return 0;
    }

    answers_load("answers.txt");

    /* One fact the program can check by compiling: how many bytes the field has. */
    {
        char id[ID_SIZE];
        answer_int("buffer.size", (long)sizeof id);
    }

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("access.kind", UINT32_C(0xdff6784c));
    answer_hash("access.size", UINT32_C(0xf53d19ac));
    answer_hash("buffer.name", UINT32_C(0xfad5942e));
    answer_hash("buffer.where", UINT32_C(0x6336598b));
    answer_hash("fault.file", UINT32_C(0x79809b62));
    answer_hash("fault.line", UINT32_C(0x2b2c916e));
    answer_hash("decl.line", UINT32_C(0x37664ab2));
    answer_hash("frame1.function", UINT32_C(0x9a633851));

    return answers_done("s00-e04-missing-nul");
}
