# s04-e01-my-strcat-strncat `[WRITE]` (S)

## Goal

Write the library's two appending functions: `my_strcat`, which appends a whole string, and
`my_strncat`, which appends at most `n` characters and then always a terminator. Afterwards
you can say exactly how many bytes each one writes, you know what the `n` of `strncat` bounds
— and, more to the point, what it does not — and you have felt why appending in a loop with
these functions is quadratic.

## Background

Appending is copying with one extra step: find the end of what is already there. Both
functions begin with that walk, and both then write from the terminator onwards. The walk is
invisible in the call and it is the reason `strcat` in a loop is the classic C performance
bug (e03 of this unit makes you wait for it).

Neither function takes the size of the destination. `strcat(dst, src)` requires the caller to
have ensured `strlen(dst) + strlen(src) + 1` bytes — the Linux manual page states that formula
in its DESCRIPTION — and nothing checks it. `strncat(dst, src, n)` bounds the **source**: it
appends at most `n` characters from `src`, then a terminator, so it writes up to `n + 1`
bytes. The `n` has nothing to do with the destination's size, which is why
`strncat(dst, src, sizeof dst)` is a buffer overflow and not a safety measure (e04).

The asymmetry is worth stating once: `strncpy`'s `n` is the number of bytes *written* into the
destination; `strncat`'s `n` is the number of characters *taken* from the source, and the
terminator is written on top of that. Two functions with an `n` in the name, two different
meanings, and only one of them bounded by the destination.

Read **Foundations** and the `strcat` and `strncat` cards in `LESSON.md` before you start.

## The API

`mycat.h` declares:

```c
char *my_strcat(char *restrict dst, const char *restrict src);
char *my_strncat(char *restrict dst, const char *restrict src, size_t n);
```

**`my_strcat(dst, src)`** appends the string `src` to the string `dst` and returns `dst`.

- `dst` must be a string with room for `strlen(dst) + strlen(src) + 1` bytes. `src` must be a
  string. The two must not overlap — that is what `restrict` promises.
- It writes `strlen(src) + 1` bytes, starting at `dst`'s old terminator. Bytes after the new
  terminator keep whatever they held.
- It reads all of `dst` (to find the end) and all of `src`.
- There is no error return and no report of anything.

**`my_strncat(dst, src, n)`** appends at most `n` characters of `src`, then a terminator.

- The same requirements, except that `dst` needs room for `strlen(dst) + n + 1` bytes — note
  the `+ 1`, which is the terminator this function always writes.
- `src` does **not** have to be a string: it may be a character sequence with at least `n`
  readable bytes. The function stops at `src`'s terminator if it finds one first, and never
  reads `src[n]`.
- It writes `min(n, strlen(src))` characters and one terminator, so between 1 and `n + 1`
  bytes, and never fewer than 1.
- It returns `dst`. There is no report of truncation.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mycat.c`, below the marked line.

## Behaviour

| Call | `dst` before | `dst` after | Bytes written |
|---|---|---|---|
| `my_strcat(dst, "def")` | `abc\0` | `abcdef\0` | 4 |
| `my_strcat(dst, "def")` | `\0` | `def\0` | 4 |
| `my_strcat(dst, "")` | `abc\0` | `abc\0` | 1 (the terminator) |
| `my_strcat(dst, "")` | `\0` | `\0` | 1 |
| `my_strcat(dst, "\xC3\xA9")` | `caf\0` | `caf\xC3\xA9\0` | 3 |
| `my_strncat(dst, "defgh", 3)` | `abc\0` | `abcdef\0` | 4 (3 + 1) |
| `my_strncat(dst, "de", 5)` | `abc\0` | `abcde\0` | 3 (stopped at the source's end) |
| `my_strncat(dst, "defgh", 0)` | `abc\0` | `abc\0` | 1 (just the terminator) |
| `my_strncat(dst, "defgh", 5)` | `\0` | `defgh\0` | 6 |
| `my_strncat(dst, "", 4)` | `abc\0` | `abc\0` | 1 |
| `my_strncat(dst, f, 4)` where `f` is `XYZW` with no terminator | `ab\0` | `abXYZW\0` | 5 |

The last row is why `src` is documented as "a string, or `n` readable bytes": a fixed-width
field can be appended with `strncat` exactly because the function stops at `n`.

## Constraints

- **Do not call** `strcat`, `strncat`, `strcpy`, `stpcpy`, `strncpy`, `stpncpy`, `strlen`,
  `strnlen`, `memcpy` or `snprintf`. The exercise is the walk and the copy; a length is not
  needed for either. The harness inspects `mycat.o`, and your file is compiled with
  `-fno-builtin`.
- One pass over `dst` to find its end, and one pass over the part of `src` you copy.
- Read nothing after `src`'s terminator; for `my_strncat`, read nothing at or after `src[n]`.
- Write nothing after the terminator you place.
- `my_strncat` must be correct for `n == 0` and for `n == SIZE_MAX`: no expression that
  computes `n + 1`.
- No allocation.

## What the harness checks

- **Five `my_strcat` cases**: an ordinary append, onto an empty string, of an empty string,
  empty onto empty, and bytes above 0x7F. Every destination is a buffer of exactly the bytes
  the append may write, so one byte too many is a fence failure in the plain build and a
  `heap-buffer-overflow` under ASan. Every source ends at its terminator.
- **Six `my_strncat` cases**: `n` smaller than the source, larger than it, exactly equal,
  `n == 0`, onto an empty string, and an empty source.
- **Two full-field cases**: a source of exactly `n` bytes with no terminator anywhere, so a
  read of `src[n]` is reported.
- **The byte count**: after `my_strncat(dst, "cdef", 2)` on `ab`, the buffer must be `abcd\0`
  and the fences intact — `n + 1` bytes, not `n`, and not `n + 2`.
- **Oracle**: 3000 generated pairs of strings with a generated `n`, compared with libc's
  `strcat` and `strncat` over the whole destination buffer including the bytes past the
  terminator.
- **Forbidden calls**: `mycat.o` references none of the ten functions listed above.

## Verify

```bash
make check
```

Success ends with `s04-e01-my-strcat-strncat: all checks passed` and `== check: OK`.

The failure that catches most first attempts is forgetting that `my_strncat` always
terminates:

```
FAIL main.c:35: my_strncat appends at most n characters
       got:  abcdef#
       want: abcdef\0
       first difference at index 6 of 7
```

Byte 6 is still `#`, so the characters were appended and the terminator was not. `strncat`
writes it unconditionally — even when `n` is 0, even when the source is empty — which is the
one thing it does that `strncpy` does not.

In the ASan stage, a `READ` report means a source was read past its bound (usually
`src[i] != '\0'` tested before `i < n`), and a `WRITE` report means the terminator went one
byte too far.

<details><summary>Hint</summary>

Both functions start with the same loop, and it is the one from S00 drill r01.

</details>
