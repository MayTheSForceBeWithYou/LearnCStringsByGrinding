# s05-e05-silent-corruption `[PREDICT]` `[NO-ASAN]`

## Goal

An unbounded copy overflows one field of a struct into the next. Predict eleven things about
the bytes before running, in a program built **without** the sanitizers. Afterwards you know
the one overflow AddressSanitizer cannot see, you can say why, and you have watched a
`role` field become `admin` without a single tool complaining.

## Background

ASan works by surrounding each **object** with poisoned redzones. A local variable, a heap
block, a global: each one is an object, and an access that lands in a redzone is reported. A
struct is **one object**, and its members are not separated by anything:

```c
typedef struct { char user[8]; char role[8]; } Account;   /* 16 bytes, one object */
```

A `strcpy` of a 13-character name into `user` writes 14 bytes. All 14 are inside the 16-byte
object. No redzone is touched, so there is nothing for ASan to report — and nothing for
UBSan, and nothing for the compiler, which cannot see the length of a name that arrived as a
parameter. The program runs to completion and prints a plausible result.

What it does instead of crashing is overwrite the neighbour. `role` held `"guest"`; after the
copy it holds whatever the tail of the name put there. If the name was chosen by someone who
had read the struct definition, `role` now holds `"admin"`, and the function that checks it
says yes. That is not a contrived example: it is the shape of a whole class of real
vulnerabilities, and it is why "ASan is clean" means "no access left its object", not "no
field was overwritten".

This exercise is built with `TAG = PREDICT NO-ASAN`, so `make check` runs the plain build
only. That is deliberate: the absence of a report is the lesson, and an ASan build would make
no difference anyway.

Read **What the tools say: nothing at all** in `LESSON.md` before you start.

## The setup

```c
typedef struct { char user[USER_SIZE]; char role[ROLE_SIZE]; } Account;   /* 8 and 8 */

void account_init(Account *a);                       /* user = "", role = "guest" */
void account_set_user(Account *a, const char *name); /* strcpy, with no bound      */
int account_is_admin(const Account *a);              /* strcmp(a->role, "admin")   */
```

The copy is `strcpy`, deliberately, and it is behind a one-line helper that takes the
destination as a parameter so that the compiler has nothing to say about it.

The program runs four cases and asks you about each:

| Case | Name copied into `user` |
|---|---|
| fits | `"ada"` (3 characters) |
| exact | `"abcdefg"` (7 characters — the longest that fits) |
| over by one | `"abcdefgh"` (8 characters) |
| attack | `"mallory_admin"` (13 characters) |

For each it asks for some combination of the struct's bytes, the string you would read out of
a field, and what `account_is_admin` says.

## How to write an answer

- **`.bytes`**: all sixteen bytes of the struct in byte notation, no spaces — a character as
  itself, a zero byte as `\0`.
- **`.user`, `.role`**: the string you would get by reading that field with `%s` or `strcmp`.
  Remember that reading a field stops at the first zero byte, wherever that is.
- **integers**: in decimal; `1` for yes and `0` for no.

Two examples that are not part of the graded set:

| | Answer |
|---|---|
| a 2-character name: the whole struct | `ab\0\0\0\0\0\0guest\0\0\0` |
| `offsetof(Account, user)` | `0` |

## Constraints

- Fill in all eleven lines before the first run. Draw the sixteen boxes for each case, mark
  which eight are `user` and which eight are `role`, and write the bytes in.
- Do not compile anything to find out, and do not add a `printf` to `main.c`.
- For the over-by-one case, count the bytes `strcpy` writes — including the one you cannot
  see.
- For the attack case, work out where the terminator lands before deciding what `role`
  contains.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. All eleven are
computed by running the code — the layout facts with `sizeof` and `offsetof`, the rest by
performing the copies — so none of them is hashed. `make check` passes only when all eleven
are `ok`.

The program has no ASan stage, because the Makefile's `TAG` says `NO-ASAN`. `make check`
prints `== plain+dsalloc` and `== check: OK`, and nothing else.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e05-silent-corruption: 0 of 11 answers correct
```

When you are done it ends with `s05-e05-silent-corruption: all checks passed`.

Afterwards, two experiments worth five minutes. Copy the program to a scratch directory
outside the tree and build it **with** ASan (`make asan` after removing `NO-ASAN` from the
Makefile): the report is still silent, which is the point. Then make `user` and `role` two
separate heap blocks instead of two members of a struct, and watch the same copy become a
`heap-buffer-overflow` immediately.

<details><summary>Hint</summary>

`strcpy` writes `strlen(src) + 1` bytes, and the last one is the terminator. For the attack
case, which byte of the struct is that?

</details>
