# s01-e03-empty-wraps: notes

## Why it is correct

The fix is to stop asking for a length:

```c
for (size_t i = 0; s[i] != '\0'; i++)
```

The loop's stopping condition is now the convention itself — the terminator — so there is no
subtraction, no empty-string special case, and no second pass over the string. The empty
string works because `s[0]` is the terminator and the body never runs; a 5-character string
works because the body runs exactly five times. The two functions in the file are now the
same shape, which is the point: this loop is the one to write by reflex.

It also fixes the cost. The defective version called `strlen(s)` in the condition, so counting
over an `n`-character string walked it `n + 1` times; the fixed version reads each byte once.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | `s[0] != '\0'` is false at once; the count is 0 and nothing else is read |
| a 1-character string | the body runs once, then the terminator ends the loop |
| bytes above 0x7F | the range tests `s[i] >= 'A' && s[i] <= 'Z'` are false for them on both platforms, because a byte above 0x7F is either negative (signed `char`) or greater than `'z'` (unsigned `char`) |
| the terminator | read, compared, never counted: the loop ends before the body |
| a very long string | one pass, no `strlen` in the condition |

The high-byte row deserves a second look, because it is luck rather than design: the
comparison happens to be false under both signednesses of `char`. A class test that has to be
right for every byte casts to `unsigned char` first, and a program that wants letters outside
ASCII needs `<ctype.h>` and a locale — S07 is that story, and S07 e01 is the version of this
same loop that breaks on a 0xFF byte.

## Alternatives

A length-based loop written safely, when you really do have a length in hand:

```c
size_t n = strlen(s);
for (size_t i = 0; i < n; i++)      /* strictly less than: no subtraction */
```

`i < n` is the form to use whenever a length is available: it is correct for `n == 0` without
a special case, and it is why "half-open ranges" (`[0, n)`) are the convention in C, in the
standard library's interfaces, and in the ASan reports you have been reading
(`[32, 36) 'id'`).

To walk a string *backwards*, the two safe shapes are

```c
for (size_t i = n; i-- > 0; )        /* decrement in the condition */
for (const char *p = s + n; p-- > s; )
```

both of which work for `n == 0`. The one that does not is `for (size_t i = n - 1; i >= 0;
i--)`: an unsigned value is always `>= 0`, so the loop never ends, and gcc says so under
`-Wtype-limits`.

The library call that replaces the whole exercise does not exist for "count upper-case
letters", but `strspn`/`strcspn` (S09) answer the related question "how long is the run of
characters from this set", and they take no length and have no empty-string case either.

## The usual mistake

Adding a special case instead of changing the loop:

```c
if (s[0] == '\0')
    return 0;
for (size_t i = 0; i <= strlen(s) - 1; i++)
```

It passes every test. It also leaves the defect in the file for the next person to copy, keeps
the quadratic cost, and will be wrong again the first time somebody writes
`i <= strlen(s) - 2` to skip a trailing character. The shape, not the input, was the problem.

The second mistake is casting: `(ptrdiff_t)strlen(s) - 1` or `(int)strlen(s) - 1`. Both make
the arithmetic signed and the loop correct for the empty string, and both convert a value
whose type was chosen to hold the size of any object into one that cannot hold it. On a string
longer than `INT_MAX` the `int` version is undefined behaviour, which is a worse bug than the
one being fixed.

## How libc does it

Nothing in libc counts character classes for you, but the library's own interfaces show the
convention this fix follows. Every length-taking function in `<string.h>` uses a count of
bytes, never a last index: `memchr(s, c, n)`, `strncmp(a, b, n)`, `strnlen(s, maxlen)`. There
is no function in the library that takes "the index of the last byte", because that
representation has no way to say "empty" — which is exactly the hole the defect fell into.

The other half of the convention is that `size_t` is unsigned on purpose: it must be able to
describe the largest object the implementation supports, and a signed type would waste half
its range. The cost of that choice is this exercise, and it is why experienced C programmers
read every `- 1` next to a `size_t` as a question.
