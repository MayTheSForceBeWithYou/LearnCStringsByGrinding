# s03-e07-struct-neighbour: answer

## The defect

`account.c`, in `account_set_user`:

```c
strcpy(a->user, name);
```

There is no check that `name` fits in the 16-byte `user` field, and the function returns 0
unconditionally although its contract promises -1 for a name that does not fit.

## The mechanism

Before, with user `fifteen_chars_x` and role `guest` (`|` marks the field boundary):

```
fifteen_chars_x\0|guest\0\0\0
```

After `account_set_user(&a, "mallory_________admin")`:

```
mallory_________|admin\0\0\0
```

`strcpy` wrote 22 bytes starting at `a->user`: 16 characters into `user`, then `admin` and
the terminator into the first 6 bytes of `role`. `user` no longer contains a terminator;
read as a string it is the whole 21-character name. `role` reads as `"admin"`.

## How the tool showed it

It did not. AddressSanitizer checks accesses against *objects*: it puts redzones around the
24-byte `Account` and reports any access that touches one. Every byte this `strcpy` wrote is
inside the `Account`, so no redzone was touched. ASan does not know where one member ends and
the next begins. If `user` were a 16-byte array of its own, the same call would be a
`stack-buffer-overflow`.

The defect was caught by the harness: `check_str(a.role, "guest", ...)` at `main.c:28` and
`main.c:33`, and `check(!account_is_admin(&a), ...)` at `main.c:34`.

## The rule you take away

A clean ASan run says no access left its object; it says nothing about one field of a struct
overflowing into the next, so lengths must be checked in code before writing into a member.
