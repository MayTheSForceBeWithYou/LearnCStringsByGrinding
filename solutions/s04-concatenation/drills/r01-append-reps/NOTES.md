# s04-r01-append-reps: notes

## Why it is correct

`app_room` is arithmetic on two pointers with the terminator reserved:

```c
return p < end ? (size_t)(end - p) - 1 : 0;
```

The `p < end` test is what keeps the subtraction from wrapping when `p` has reached `end`.
Nothing is dereferenced, so an appender can ask before every write.

`app_str` copies `min(strlen(s), room)` bytes and terminates:

```c
memcpy(p, s, keep);
p += keep;
*p = '\0';
return p;
```

`*p = '\0'` is always inside the buffer, because `keep` was bounded by `room`, which already
reserved that byte. When nothing fits, `keep` is 0 and the function rewrites the terminator
where it was — correct, and the reason a truncated chain stays a valid string.

`app_char` is the same with a length of one, and returns `p` unchanged when there is no room,
which is what makes `app_dec`'s loop able to stop.

`app_dec` generates digits in reverse into a small array and appends them through `app_char`:

```c
if (v < 0) { p = app_char(p, end, '-'); u = 0UL - (unsigned long)v; }
else        u = (unsigned long)v;
do { digits[n++] = '0' + u % 10; u /= 10; } while (u > 0);
while (n > 0) { char *q = app_char(p, end, digits[--n]); if (q == p) break; p = q; }
```

Three things are load-bearing. The conversion to `unsigned long` **before** negating handles
`LONG_MIN`: `-LONG_MIN` is undefined (it does not fit in a `long`), while
`0UL - (unsigned long)v` is defined modular arithmetic and gives exactly the magnitude.
The `do`/`while` emits one digit for 0. And the append loop stops as soon as `app_char`
reports no progress, so a truncated number keeps its leading digits rather than its trailing
ones — `12345` into four bytes is `123`, which is what a reader would expect.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `p == end` | `app_room`'s `p < end` test: 0, and no subtraction |
| a one-byte buffer | `room` is 0 everywhere; every appender writes only the terminator |
| an exact fit | `keep == strlen(s)`, and the terminator goes in the last byte |
| a truncated append | `keep == room`; `p` ends at `end - 1` |
| appending after truncation | `room` is 0, so nothing more is written |
| `v == 0` | the `do`/`while` runs its body once |
| a negative `v` | the sign is appended first, so it survives truncation |
| `LONG_MIN` | `0UL - (unsigned long)v`, never a negation |
| bytes above 0x7F | nothing inspects a byte's value |

## Alternatives

The same four with an index and a capacity (`buf`, `cap`, `len`) instead of two pointers is
S04 e05's cursor, and the two are interchangeable: `p` is `buf + len` and `end` is
`buf + cap`. Pointers are slightly tidier inside one function, an index slightly tidier when
the length is also part of the interface.

With the library, each of the three appenders is one call:

```c
int n = snprintf(p, (size_t)(end - p), "%s", s);     /* or "%c", or "%ld" */
p += n < (int)(end - p) ? n : (end - p) - 1;
```

Correct, bounded, and it parses a format string per call — about twenty times the cost of the
`memcpy` version per append. `snprintf` is the right tool when the piece needs formatting and
the wrong one when it is already a string. `app_dec` exists in this drill because producing
digits by hand is a skill S11 e04 needs in full (`u64_to_str` with bases and `INT64_MIN`).

The Linux `string_copying(7)` page sketches exactly this interface under the name `stpecpy`:
an appender that takes `dst` and `end` and returns the new end. It is a proposal, not
something you can call, and the shape is worth knowing because it is what careful C code
converges on.

## The usual mistake

Negating `LONG_MIN`. `-v` for `v == LONG_MIN` is undefined behaviour — the result does not fit
in a `long` — and UBSan reports it as a signed integer overflow. The fix is to do the
arithmetic in `unsigned long`, where wrapping is defined: `0UL - (unsigned long)v` is the
magnitude for every `v`, including the one that has no positive counterpart.

Second: forgetting the terminator when nothing fits, so that a truncated chain leaves a buffer
that is not a string. Every appender here writes `*p = '\0'` on every path.

Third: `app_room` written as `end - p` without reserving the terminator. Every append then
fills the last byte with a character and puts the terminator at `end`, one past the buffer —
a fence failure in the plain build and an ASan `WRITE` report.

Fourth: in `app_dec`, appending the digits in the order they were generated. They come out
least-significant first; the loop has to walk the array backwards.

## How libc does it

There is no end-pointer interface in the C library: every function takes a pointer and a size,
or a pointer and a terminator. The closest relatives are `stpcpy` and `stpncpy` (S03), which
*return* the end of what they wrote so that a caller can chain — exactly the property this
drill's four functions have.

For the decimal conversion, glibc's `printf` family produces digits into a buffer from the
least significant end, using a division-free routine for the common cases (multiply-and-shift
instead of `% 10`), and then copies the digits into place. The structure of `app_dec` — generate
in reverse, then emit — is the same one, and it is the only sensible one: you cannot know how
many digits there are until you have produced them all.
