/* account.c -- s03-e07-struct-neighbour. Fixed: see NOTES.md. */
#include "account.h"

#include <string.h>

void account_init(Account *a)
{
    memset(a, 0, sizeof *a);
    strcpy(a->role, "guest");
}

int account_set_user(Account *a, const char *name)
{
    size_t len = strlen(name);
    if (len >= sizeof a->user) /* the terminator needs a byte too */
        return -1;
    memcpy(a->user, name, len + 1);
    return 0;
}

int account_is_admin(const Account *a)
{
    return strcmp(a->role, "admin") == 0;
}
