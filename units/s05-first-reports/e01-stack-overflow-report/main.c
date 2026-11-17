/* s05-e01-stack-overflow-report: read the report, then answer. */

/* With no arguments this program is only the grader for answers.txt. With the argument
 * `bad` it copies a name into a buffer that is too small for it, which is how
 * capture.txt was made:
 *
 *     make asan ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "badge.h"

/* The copy, through a parameter, so that nothing about it is visible to the compiler. */
static void copy_into(char *dst, const char *src)
{
    strcpy(dst, src);
}

int main(int argc, char **argv)
{
    char tag[8];

    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        copy_into(tag, "Dodecahedron");
        puts(tag);
        badge_print(tag);
        return 0;
    }

    answers_load("answers.txt");

    /* One fact the program can state by compiling. */
    answer_int("buffer.size", (long)sizeof tag);

    /* The rest are facts about the report and the source, compared by hash. */
    answer_hash("access.kind", UINT32_C(0xefe59d87));
    answer_hash("access.size", UINT32_C(0x16256ba9));
    answer_hash("buffer.name", UINT32_C(0xcef6ff41));
    answer_hash("buffer.where", UINT32_C(0x6336598b));
    answer_hash("decl.line", UINT32_C(0xc55edb77));
    answer_hash("fault.file", UINT32_C(0x79bb65ce));
    answer_hash("fault.line", UINT32_C(0x2b2c916e));
    answer_hash("frame1.function", UINT32_C(0xeba20e70));
    answer_hash("overflow.bytes", UINT32_C(0x2339ecc0));

    return answers_done("s05-e01-stack-overflow-report");
}
