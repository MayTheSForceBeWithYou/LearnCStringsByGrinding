# s03-e06-copy-policy: notes

## Why it is correct

All three functions have the same skeleton: handle `cap == 0`, obtain the source length,
make one comparison `len < cap` (or `n < cap`), and only then write.

- `copy_trunc` stores `min(len, cap - 1)` characters with one `memcpy` and a terminator, and
  reports `len < cap`.
- `copy_strict` writes either `len + 1` bytes (the terminator comes along in the `memcpy`) or
  the single byte `dst[0] = '\0'`.
- `copy_n` never calls `strlen`: the length is given. `memcpy(dst, src, n)` reads exactly `n`
  bytes, and the terminator is written separately because the source has none.

The comparison in `copy_n` is `n >= cap`, not `n + 1 > cap`. They mean the same thing for
every `n` except `SIZE_MAX`, where `n + 1` is 0 and the second form would report "fits" and
then copy `SIZE_MAX` bytes.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the first `if` in each function; `dst` is never dereferenced |
| `cap == 1` | `copy_trunc`: `n = 0`, terminator at index 0; the others: only `""` fits |
| exact fit (`len == cap - 1`) | `len < cap` is true |
| one too many (`len == cap`) | `len < cap` is false |
| `copy_n` with `n == 0` | `memcpy(dst, src, 0)` reads nothing; `dst[0] = '\0'` |
| NUL among `copy_n`'s bytes | `memcpy` does not look at the bytes it copies |
| `copy_n` with `n == SIZE_MAX` | `n >= cap` is true; nothing is copied |

## The contracts

`copypol.h` in this directory has reference answers to the nine questions. The ones people
most often leave unanswered:

- *Bytes after the terminator.* "Never written" is a promise callers can use (a buffer
  reused for shorter and shorter strings does not get cleared, and does not need to be).
- *Why `""` on failure.* The alternative, "leave `dst` unchanged", sounds safer and is worse:
  a caller that forgets the check then proceeds with the *previous* value, which is valid,
  plausible and wrong. An empty string fails loudly and early.
- *NULL.* Saying "must not be NULL; not checked" is a complete answer. Saying nothing is not.

## Alternatives

`copy_trunc` is `snprintf(dst, cap, "%s", src)` with the return value mapped to the three
codes. `copy_n` could be `snprintf(dst, cap, "%.*s", (int)n, src)`, but that stops at an
embedded NUL, needs `n` to fit in an `int`, and is slower; `memcpy` is the honest tool.

A fourth policy exists: "truncate, and return the needed size" (the `snprintf`/`strlcpy`
contract), which lets the caller allocate and retry. It is the right one when a retry is
possible.

## The usual mistake

Writing first and deciding afterwards: copying `cap - 1` characters in `copy_strict`,
noticing the source has more, and then "undoing" with `dst[0] = '\0'`. It passes the checks
here, and it means that for a moment the buffer held a truncated identifier. In a function
that another thread or a signal handler can observe, or one that is later changed to return
early, that moment is the bug.

## How libc does it

It does not: the library has no "refuse" copy, which is the reason for the exercise. The
nearest standard design is C11 Annex K's `strcpy_s`, which on failure sets `dst[0]` to NUL
and calls a constraint handler. Almost no libc implements it (S19).
