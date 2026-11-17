/* s05-e06-fortify-abort: read the report, then answer. */

/* This exercise is built with -O2 -D_FORTIFY_SOURCE=2 and WITHOUT the sanitizers, so
 * that the check that stops it is glibc's own, not AddressSanitizer's.
 *
 * With no arguments the program is only the grader for answers.txt. With the argument
 * `bad` it greets a name that does not fit, which is how capture.txt was made:
 *
 *     make run ARGS=bad
 *
 * Fill in answers.txt from capture.txt and the source. Then `make check`.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"
#include "greet.h"

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "bad") == 0) {
        greet_print("Bartholomew"); /* 11 characters: "Hello, Bartholomew!" needs 20 bytes */
        return 0;
    }

    /* A name that fits. */
    greet_print("Ada");

    answers_load("answers.txt");

    /* Two facts the program can state by compiling. */
    answer_int("buffer.size", GREET_SIZE);
    answer_int("fits.longest", (long)(GREET_SIZE - 1 - (long)strlen("Hello, !")));

    /* The rest are facts about the run and the source, compared by hash. */
    answer_hash("stopped_by", UINT32_C(0x174f9489));
    answer_hash("exit.signal", UINT32_C(0x47f210c4));
    answer_hash("exit.status", UINT32_C(0xa08b15a6));
    answer_hash("names.line", UINT32_C(0x10115acb));
    answer_hash("names.buffer", UINT32_C(0xaa1b9edb));
    answer_hash("asan_would_say", UINT32_C(0xf57b98eb));

    return answers_done("s05-e06-fortify-abort");
}
