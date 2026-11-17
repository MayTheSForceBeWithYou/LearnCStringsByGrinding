# s02-r01-gap-reps `[DRILL]` (XS)

## Skill

Moving bytes around **inside one buffer**: open a gap, close a gap, delete a range, insert
some bytes. Every one of these is a `memmove` with two offsets and a count, and getting the
count right is the drill.

This is the operation under every text editor, every dynamic array's `insert` and `erase`,
every packet header rewrite and every in-place string edit you will write for the rest of the
course (S03's copies, S04's appenders, S11 e07's escaper, S19's owned string). It is also the
operation that people reach for `memcpy` for, which is undefined behaviour and usually works —
e03 of this unit is a program that did it.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `gap.c`, below the marked line.

## Functions

```c
size_t gap_open(char *buf, size_t cap, size_t len, size_t at, size_t n);
size_t gap_close(char *buf, size_t cap, size_t len, size_t at, size_t n);
size_t gap_delete(char *buf, size_t cap, size_t len, size_t from, size_t to);
size_t gap_insert(char *buf, size_t cap, size_t len, size_t at, const char *src, size_t n);
```

All four work on a buffer of `cap` bytes whose first `len` bytes are in use. There is **no
terminator**: this drill is about counts. Each returns the new used length, or `len` unchanged
if the request does not fit.

| Function | Does | Returns `len` unchanged when |
|---|---|---|
| `gap_open` | the bytes `[at, len)` move up by `n`; the gap's own bytes are not written | `at > len`, or `len + n > cap` |
| `gap_close` | the bytes `[at + n, len)` move down by `n`; the `n` bytes at the end are no longer used and are not cleared | `at + n > len` |
| `gap_delete` | removes the range `[from, to)` | `from > to`, or `to > len` |
| `gap_insert` | the `n` bytes at `src` (which does not overlap `buf`) go in at `at` | `at > len`, or `len + n > cap` |

In a 12-byte buffer holding `abcdef######` (six used bytes, `#` for unused):

| Call | Buffer afterwards | Returns |
|---|---|---|
| `gap_open(buf, 12, 6, 3, 2)` | `abcdedef####` | 8 |
| `gap_open(buf, 12, 6, 0, 2)` | `ababcdef####` | 8 |
| `gap_open(buf, 12, 6, 6, 2)` | `abcdef######` | 8 |
| `gap_close(buf, 12, 6, 2, 2)` | `abefef######` | 4 |
| `gap_close(buf, 12, 6, 0, 6)` | `abcdef######` | 0 |
| `gap_delete(buf, 12, 6, 1, 3)` | `adefef######` | 4 |
| `gap_insert(buf, 12, 6, 3, "XY", 2)` | `abcXYdef####` | 8 |

The first row is the one to work out on paper. Opening a two-byte gap at offset 3 moves three
bytes (`def`) up to offsets 5, 6 and 7; offsets 3 and 4 keep what they had, which was `de`. A
gap is *room*, not cleared space — the caller is about to write over it, and clearing it would
be a second pass.

The rows that return early matter as much as the ones that move: a request that does not fit
must change **nothing**, so every test comes before every move.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s02-mem-functions/drills/r01-gap-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- `memmove` and `memcpy` are the tools; `memccpy`, `strcpy`, `stpcpy`, `strncpy`, `strcat`,
  `strncat` and `snprintf` are in `FORBID`. Use the right one of the first two in each place
  and be able to say why.
- Every arithmetic expression must be correct for the extremes: `n == 0`, `at == len`,
  `len == 0`, `len == cap`. Watch for subtractions on `size_t` — `cap - len` and
  `len - at - n` both wrap if the test that protects them is missing or in the wrong order.
- `gap_delete` may call `gap_close`, and `gap_insert` may call `gap_open`. That is the
  intended shape.

## Target

12 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs twenty-six cases: each function in the middle, at the front and at the end of
the buffer; with `n == 0`; with a request that exactly fills the buffer; with one that is one
byte too large; with offsets past the end; and `gap_delete` with `from > to`. Every buffer is
exactly `cap` bytes with fences around it, so a move that writes one byte too far is caught in
the plain build and reported by ASan in the sanitizer build. It prints
`s02-r01-gap-reps: all checks passed`.

A `memcpy-param-overlap` report in the ASan stage means `memcpy` where `memmove` was needed —
the two regions are both inside `buf`. A failure whose `got` line shows the right bytes in the
wrong place is almost always a count: `len - at` and `len - at - n` are the two that are easy
to mix up.
