/* oracle.h -- deterministic generated inputs for differential ("oracle") tests.
 *
 * A reimplementation harness runs your function and the libc function on the same
 * generated strings and compares the results. The generator is a small linear
 * congruential generator, never rand(): the same seed gives the same strings on every
 * platform, so a failure message's "seed 7, case 1234" reproduces everywhere.
 *
 *     orc_seed(7);
 *     for (int i = 0; i < 2000; i++) {
 *         size_t n = orc_string(src, 24, ORC_ALPHA_HIGH);
 *         ... call both, compare, on mismatch print orc_case() ...
 *     }
 */
#ifndef COURSE_ORACLE_H
#define COURSE_ORACLE_H

#include <stddef.h>
#include <stdint.h>

/* Two letters: partial matches and repeats are common (searching, comparing). */
#define ORC_ALPHA_AB "ab"
/* Letters, digits, punctuation and white space. */
#define ORC_ALPHA_TEXT "abcxyzABC019 _-.,/\t\n"
/* Includes bytes above 0x7F, which are negative in a signed char. */
#define ORC_ALPHA_HIGH "abz AZ09\x80\xE9\xFF\x01"

void orc_seed(uint32_t seed);

/* The next 32-bit value of the sequence. */
uint32_t orc_next(void);

/* A value in 0 .. n-1 (n > 0). */
uint32_t orc_below(uint32_t n);

/* Writes a string of 0 .. maxlen characters drawn from alphabet into buf, plus its
 * terminator (buf needs maxlen + 1 bytes), and returns the length. About one string in
 * eight is empty and one in eight has the full length, because those are the cases
 * that break code. */
size_t orc_string(char *buf, size_t maxlen, const char *alphabet);

/* How many strings orc_string has produced since the last orc_seed: the case number
 * to print when a comparison fails. */
uint32_t orc_case(void);

#endif
