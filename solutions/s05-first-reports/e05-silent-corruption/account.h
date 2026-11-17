/* account.h -- s05-e05-silent-corruption: two text fields, side by side. */
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stddef.h>

#define USER_SIZE 8
#define ROLE_SIZE 8

typedef struct {
    char user[USER_SIZE];
    char role[ROLE_SIZE];
} Account;

/* Sets user to "" and role to "guest". */
void account_init(Account *a);

/* Copies `name` into the user field with strcpy -- no bound, on purpose: this
 * exercise is about what happens when the name does not fit. */
void account_set_user(Account *a, const char *name);

/* 1 if the account's role is exactly "admin", else 0. */
int account_is_admin(const Account *a);

#endif
