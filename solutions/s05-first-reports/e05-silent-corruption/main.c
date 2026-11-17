/* s05-e05-silent-corruption: predict what an overflow inside one struct does.
 *
 * This program is built WITHOUT the sanitizers (the Makefile's TAG says NO-ASAN),
 * because the whole point is what the tools do not report. It performs an unbounded
 * copy into an 8-byte field of a 16-byte struct and then asks what the neighbouring
 * field holds.
 *
 * Fill in answers.txt BEFORE you run this.
 */
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "answers.h"
#include "bytes.h"

int main(int argc, char **argv)
{
    Account a;

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    /* Three facts about the layout, which the compiler knows. */
    answer_int("struct.size", (long)sizeof(Account));
    answer_int("user.offset", (long)offsetof(Account, user));
    answer_int("role.offset", (long)offsetof(Account, role));

    /* A name that fits: nothing surprising. */
    account_init(&a);
    account_set_user(&a, "ada");
    answer_bytes("fits.bytes", &a, sizeof a);
    answer_str("fits.role", a.role);

    /* A name of exactly USER_SIZE - 1 characters: still fits. */
    account_init(&a);
    account_set_user(&a, "abcdefg");
    answer_str("exact.role", a.role);

    /* A name one character too long. */
    account_init(&a);
    account_set_user(&a, "abcdefgh");
    answer_bytes("over1.bytes", &a, sizeof a);
    answer_int("over1.role_len", (long)strlen(a.role));

    /* A name chosen by someone who has read the struct definition. */
    account_init(&a);
    account_set_user(&a, "mallory_admin");
    answer_str("attack.user", a.user);
    answer_str("attack.role", a.role);
    answer_int("attack.is_admin", account_is_admin(&a));

    return answers_done("s05-e05-silent-corruption");
}
