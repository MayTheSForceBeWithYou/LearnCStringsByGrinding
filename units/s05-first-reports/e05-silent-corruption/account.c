/* account.c -- s05-e05-silent-corruption. The copy here is deliberately unbounded. */
#include "account.h"

#include <string.h>

/* Through a parameter, so that the compiler cannot see the size and comment on it. */
static void copy_into(char *dst, const char *src)
{
    strcpy(dst, src);
}

void account_init(Account *a)
{
    memset(a, 0, sizeof *a);
    copy_into(a->role, "guest");
}

void account_set_user(Account *a, const char *name)
{
    copy_into(a->user, name);
}

int account_is_admin(const Account *a)
{
    return strcmp(a->role, "admin") == 0;
}
