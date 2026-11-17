/* s05-e04-compiler-warnings: which of the ten snippets does gcc warn about?
 *
 * snippets.c holds ten small functions. It is never linked and never run, and the
 * harness does not compile it. Read them, decide which ones draw a diagnostic from
 * gcc at -O2 with -Wall -Wextra -Wpedantic, and which flag each one is under, and fill
 * in answers.txt. Then `make check`.
 *
 * Nothing in this file knows the answers: they are compared by hash.
 */
#include <stdio.h>

#include "answers.h"

int main(int argc, char **argv)
{
    answers_load(argc > 1 ? argv[1] : "answers.txt");

    answer_hash("s01.warns", UINT32_C(0xc4702e4a));
    answer_hash("s02.warns", UINT32_C(0x22dc6891));
    answer_hash("s02.flag", UINT32_C(0xbf1baa83));
    answer_hash("s03.warns", UINT32_C(0xeb06092c));
    answer_hash("s04.warns", UINT32_C(0x8e9f6bb3));
    answer_hash("s04.flag", UINT32_C(0xea4ed2f3));
    answer_hash("s05.warns", UINT32_C(0x4683c686));
    answer_hash("s06.warns", UINT32_C(0xad0ece1d));
    answer_hash("s06.flag", UINT32_C(0x371b7508));
    answer_hash("s07.warns", UINT32_C(0xb1978226));
    answer_hash("s07.flag", UINT32_C(0xe00edf75));
    answer_hash("s08.warns", UINT32_C(0xb5caa353));
    answer_hash("s09.warns", UINT32_C(0xa6943938));
    answer_hash("s09.flag", UINT32_C(0x0b15ece1));
    answer_hash("s10.warns", UINT32_C(0x06ed3d6e));
    answer_hash("s10.flag", UINT32_C(0xfd941d63));

    return answers_done("s05-e04-compiler-warnings");
}
