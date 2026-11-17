# s03-e07-struct-neighbour `[BUG]`

## Goal

A login record has a 16-byte user name and, right after it, an 8-byte role. Setting a user
name that is too long changes the role. Find the defect, fix it so the function meets its
contract, and explain why AddressSanitizer, which you have learned to trust, says nothing at
all. Afterwards you know one thing ASan cannot see, and you know that "the sanitizer is
clean" is a statement about the tool, not about the code.

## Background

`strcpy` writes `strlen(src) + 1` bytes starting at the destination, and it will write them
whatever is there. When the destination is an array of its own, the bytes after it belong to
something else (a redzone, under ASan), and the tool catches the write. When the destination
is a **member of a struct**, the bytes after it are the next member of the same struct.

ASan tracks *objects*: a heap block, a stack variable, a global. An `Account` is one object.
A write that starts in `user` and ends in `role` never leaves that object, so every byte it
touches is, as far as ASan knows, addressable memory that the program owns. No redzone is
crossed and no report is made.

This is not a corner case. It is how the classic "overflow the name, overwrite the privilege
flag" vulnerability works, and it is why a struct that holds text needs functions that check
lengths, rather than tools that might notice afterwards.

Read the `strcpy` card and the section **What ASan cannot see** in `LESSON.md` first.

## Symptom

```
== plain+dsalloc
FAIL main.c:26: a 16-character name is rejected with -1
FAIL main.c:27: a rejected name leaves user unchanged
       got:  sixteen_chars_xy\0
       want: fifteen_chars_x\0
FAIL main.c:28: a rejected name leaves role unchanged
       got:  \0
       want: guest\0
...
FAIL main.c:33: a rejected name leaves role unchanged (21 characters)
       got:  admin\0
       want: guest\0
FAIL main.c:34: setting a user name never makes an account an administrator
s03-e07-struct-neighbour: 7 check(s) failed
```

Now run the sanitizer stage by itself:

```bash
make asan
```

You get the same seven `FAIL` lines and **no sanitizer report**: no `ERROR: AddressSanitizer`,
no stack trace, no abort. The program runs to the end. A buffer has been overflowed twice and
the tool that finds buffer overflows has nothing to say.

## What the code is supposed to do

`account.h` is the specification, and it is correct:

- `account_init(a)` sets `user` to the empty string and `role` to `"guest"`.
- `account_set_user(a, name)` copies `name` into `user` and returns 0. If `name` has more than
  15 characters it **returns -1 and changes nothing** in the account: `user` keeps its old
  value, and `role` is not touched in any case.
- `account_is_admin(a)` is 1 exactly when `role` is `"admin"`.

| Call | Returns | `user` afterwards | `role` afterwards |
|---|---|---|---|
| `account_set_user(&a, "alice")` | 0 | `alice` | unchanged |
| `account_set_user(&a, "fifteen_chars_x")` (15) | 0 | `fifteen_chars_x` | unchanged |
| `account_set_user(&a, "sixteen_chars_xy")` (16) | -1 | unchanged | unchanged |
| `account_set_user(&a, "mallory_________admin")` (21) | -1 | unchanged | unchanged |
| `account_set_user(&a, "")` | 0 | empty | unchanged |

Note that the contract says *reject*, not *truncate*. A login system that silently shortens a
user name has created a second user.

## What to hand in

1. **The fix**, in `account.c`. There is exactly one defect, in one function. Do not change
   `main.c` or `account.h`, and do not reorder or resize the struct's members: moving the
   victim does not remove the overflow.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted.
   - *The mechanism*: the 24 bytes of the `Account` before and after
     `account_set_user(&a, "mallory_________admin")`, in byte notation, with the boundary
     between the two fields marked. Which call wrote which bytes?
   - *How the tool showed it*: it did not. Say why, in terms of what ASan tracks, and say
     which check in `main.c` caught the defect instead.
   - *The rule you take away*: one sentence.

## Constraints

- On rejection, not one byte of the account may change.
- Decide whether the name fits **before** writing anything.
- No allocation.

## Verify

```bash
make check
```

Done means `s03-e07-struct-neighbour: all checks passed` in both stages and `== check: OK`.

<details><summary>Hint</summary>

Count the characters of `mallory_________admin` up to the `a` of `admin`.

</details>
