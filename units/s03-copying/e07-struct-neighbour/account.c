/* account.c -- s03-e07-struct-neighbour. There is one defect in this file. */
#include "account.h"

#include <string.h>

void account_init(Account *a)
{
    memset(a, 0, sizeof *a);
    strcpy(a->role, "guest");
}

int account_set_user(Account *a, const char *name)
{
    strcpy(a->user, name);
    return 0;
}

int account_is_admin(const Account *a)
{
    return strcmp(a->role, "admin") == 0;
}
