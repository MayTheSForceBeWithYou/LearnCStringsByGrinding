/* s00-e05-char-signedness: one byte, three types, eighteen answers.
 *
 * The byte is 0xE9. In Latin-1 it is the character e-acute; in UTF-8 it is the first
 * byte of a two-byte sequence; here it is just a byte above 0x7F, which is where every
 * char-signedness defect lives. The program holds it in a signed char, an unsigned
 * char and a plain char, evaluates eighteen expressions, and compares each with your
 * prediction in answers.txt.
 *
 * Fill in answers.txt BEFORE you run this.
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* A 256-entry class table of the kind real code builds: 1 for an ASCII letter. */
static int letter[256];

static void build_letter_table(void)
{
    for (int i = 'a'; i <= 'z'; i++)
        letter[i] = 1;
    for (int i = 'A'; i <= 'Z'; i++)
        letter[i] = 1;
}

/* The three expressions below are evaluated through functions, so that each one is a
 * real comparison of two values at run time and not a constant the compiler folds. */
static int eq(int a, int b)
{
    return a == b;
}

static int lt(int a, int b)
{
    return a < b;
}

static int gt(int a, int b)
{
    return a > b;
}

int main(int argc, char **argv)
{
    signed char sc = (signed char)0xE9;
    unsigned char uc = (unsigned char)0xE9;
    char pc = (char)0xE9;

    build_letter_table();
    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* What the integer promotions make of each one. */
    answer_int("sc.value", sc);
    answer_int("uc.value", uc);
    answer_int("pc.value", pc);
    answer_int("pc.is_signed", CHAR_MIN < 0 ? 1 : 0);

    /* Comparisons with zero: the "is this a high byte?" test, done two ways. */
    answer_int("sc.gt_zero", gt(sc, 0));
    answer_int("uc.gt_zero", gt(uc, 0));

    /* The same byte compared with itself in the other type, and with a character
     * constant that was written with the same two hex digits. */
    answer_int("sc.eq_uc", eq(sc, uc));
    answer_int("sc.eq_charconst", eq(sc, '\xE9'));
    answer_int("uc.eq_charconst", eq(uc, '\xE9'));

    /* Ordering against an ordinary letter: the heart of a hand-written comparison. */
    answer_int("sc.lt_a", lt(sc, 'a'));
    answer_int("uc.lt_a", lt(uc, 'a'));

    /* Casts between the two character types. */
    answer_int("sc.to_uchar", (unsigned char)sc);
    answer_int("uc.to_schar", (signed char)uc);

    /* The high nibble, which is how a hex dump is written. */
    answer_int("sc.shift4", sc >> 4);
    answer_int("uc.shift4", uc >> 4);

    /* Array indexes: the value the subscript would use. */
    answer_int("index.sc", sc);
    answer_int("index.uc", uc);
    answer_int("letter.at_uc", letter[uc]);
    answer_hash("letter.at_sc", UINT32_C(0x44d77e32));

    return answers_done("s00-e05-char-signedness");
}
