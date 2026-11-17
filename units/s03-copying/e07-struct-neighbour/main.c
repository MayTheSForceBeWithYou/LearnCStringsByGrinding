/* s03-e07-struct-neighbour: harness. The defect is in account.c, not here. */
#include <stdio.h>
#include <string.h>

#include "account.h"
#include "course.h"
#include "strcheck.h"

int main(void)
{
    Account a;

    account_init(&a);
    check_str(a.user, "", "a new account has an empty user name");
    check_str(a.role, "guest", "a new account has the role guest");

    check(account_set_user(&a, "alice") == 0, "a 5-character name is accepted");
    check_str(a.user, "alice", "user is alice");
    check_str(a.role, "guest", "role is still guest after setting a short name");

    check(account_set_user(&a, "fifteen_chars_x") == 0, "a 15-character name is accepted (it fits exactly)");
    check_str(a.user, "fifteen_chars_x", "user is the 15-character name");
    check_str(a.role, "guest", "role is still guest after a name that fits exactly");

    /* 16 characters: one too many for a 16-byte field. */
    check(account_set_user(&a, "sixteen_chars_xy") == -1, "a 16-character name is rejected with -1");
    check_str(a.user, "fifteen_chars_x", "a rejected name leaves user unchanged");
    check_str(a.role, "guest", "a rejected name leaves role unchanged");

    /* 21 characters, chosen by someone who has read the struct definition. */
    check(account_set_user(&a, "mallory_________admin") == -1, "a 21-character name is rejected with -1");
    check_str(a.user, "fifteen_chars_x", "a rejected name leaves user unchanged (21 characters)");
    check_str(a.role, "guest", "a rejected name leaves role unchanged (21 characters)");
    check(!account_is_admin(&a), "setting a user name never makes an account an administrator");

    check(account_set_user(&a, "") == 0, "an empty name is accepted");
    check_str(a.user, "", "user is empty");

    return checks_done("s03-e07-struct-neighbour");
}
