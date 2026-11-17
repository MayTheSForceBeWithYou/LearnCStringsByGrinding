/* account.h -- s03-e07-struct-neighbour: a login record with two text fields. */
#ifndef ACCOUNT_H
#define ACCOUNT_H

#define USER_SIZE 16 /* at most 15 characters and a terminator */
#define ROLE_SIZE 8

typedef struct {
    char user[USER_SIZE];
    char role[ROLE_SIZE]; /* "guest" until an administrator changes it */
} Account;

/* Sets user to "" and role to "guest". */
void account_init(Account *a);

/* Sets the user name. `name` is borrowed.
 * Returns 0 on success. If `name` does not fit in the user field (more than
 * USER_SIZE - 1 characters), returns -1 and changes nothing in *a. */
int account_set_user(Account *a, const char *name);

/* 1 if the account's role is exactly "admin", else 0. */
int account_is_admin(const Account *a);

#endif
