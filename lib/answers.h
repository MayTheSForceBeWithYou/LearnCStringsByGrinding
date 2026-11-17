/* answers.h -- grades answers.txt for [PREDICT], [LOOKUP] and [REPORT] exercises.
 *
 * answers.txt, next to the program:
 *
 *     # comment lines and blank lines are ignored
 *     call01.buf = hello\0\0\0
 *     call01.ret = 0
 *
 * One answer per line, "key = value". White space around the key and the value is
 * ignored. A key with nothing after the "=" is blank.
 *
 * The program computes each true value by running the code in question, then asks:
 *
 *     answers_load(argc > 1 ? argv[1] : "answers.txt");
 *     answer_bytes("call01.buf", buf, 8);
 *     answer_int("call01.ret", ret);
 *     return answers_done("s03-e04-strncpy-predict");
 *
 * Each key is reported as ok, WRONG or blank. The true value is never printed.
 * answers_done() returns 0 only when every key is ok.
 */
#ifndef COURSE_ANSWERS_H
#define COURSE_ANSWERS_H

#include <stddef.h>
#include <stdint.h>

/* Reads the file. Returns 0, or -1 (after a message) if it cannot be opened or a
 * line is not "key = value". */
int answers_load(const char *path);

/* The answer must equal truth exactly (after trimming white space). */
void answer_str(const char *key, const char *truth);

/* The answer must be a decimal integer equal to truth. */
void answer_int(const char *key, long truth);

/* The answer must be the n bytes at p in byte notation (lib/bytes.h). */
void answer_bytes(const char *key, const void *p, size_t n);

/* For facts that cannot be computed (looked up in a manual page): the answer, trimmed
 * and lower-cased, must hash to `hash`: answers_hash("<key>=<answer>"). The source then does not
 * give the answer away, and equal answers to different questions hash differently. */
void answer_hash(const char *key, uint32_t hash);

/* 32-bit FNV-1a of s after trimming and lower-casing. Authors: run the program with
 * ANSWERS_SHOW_HASH=1 in the environment and it prints the hash of each answer given
 * in answers.txt next to its verdict. */
uint32_t answers_hash(const char *s);

/* Prints the summary; "<id>: all checks passed" and 0 if every key was ok, else 1.
 * Keys in the file that the program never asked about are listed as unknown and
 * count as wrong (usually a typo in the key). */
int answers_done(const char *id);

#endif
