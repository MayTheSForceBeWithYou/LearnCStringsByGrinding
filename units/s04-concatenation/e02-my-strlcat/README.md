# s04-e02-my-strlcat `[WRITE]` (S)

## Goal

Write `strlcat`, the BSD append that takes the destination's **size** instead of a character
count and reports the length it tried to create. Afterwards you know the one appending
contract in common use that a caller cannot get wrong by accident, you can detect truncation
with a single comparison, and you have handled the case that its specification spends most of
its words on: a destination that is not a string.

## Background

`strcat` takes no bound at all; `strncat`'s bound is on the *source*, which is almost never
what the caller knows. What a caller does know is `sizeof dst`. `strlcat(dst, src, size)` takes
exactly that, and its return value is the length of the string it *wanted* to create, so:

```c
if (strlcat(path, name, sizeof path) >= sizeof path)
    /* truncated: the result needed ret + 1 bytes */
```

One call, one comparison, and the destination is always a string afterwards. That is the whole
design, and it is why the function spread from OpenBSD (1998) to the other BSDs, to macOS, to
glibc 2.38, and finally into POSIX.1-2024 — too late for the standards this course compiles
against, which is why you write it rather than call it.

The interesting part of the contract is the case where `dst` has no terminator among its
`size` bytes. `strcat` and `strncat` are simply undefined there. `strlcat` is defined: it
appends nothing and returns `size + strlen(src)` — a number greater than or equal to `size`,
so the caller's truncation test still reports a problem. It is a small thing and it is the
reason the function can be handed a buffer from somewhere else without a prior check.

The other thing worth noticing is the cost. `strlcat` reads **all** of `src` even when none of
it fits, because the return value is a length it must compute. That is a real trade: the
function cannot be used safely on a source of unknown length in a hot loop, and e03 of this
unit is about what repeated appending costs.

Read **Foundations** and the `strlcat` card in `LESSON.md` before you start.

## The API

`mystrlcat.h` declares:

```c
size_t my_strlcat(char *dst, const char *src, size_t size);
```

- `src` is a string. `dst` has `size` readable and writable bytes; `size` may be 0, and `dst`
  does **not** have to be terminated.
- **Writes**: never more than `size` bytes in all, and never `dst[size]` or beyond. When
  anything is written, the result is terminated. Nothing at all is written when `size == 0`,
  or when `dst` holds no terminator among its `size` bytes.
- **Reads**: at most `size` bytes of `dst`, and all of `src` — every time, even when nothing
  fits.
- **Returns**: the length of the string it tried to create, which is `strlen(dst) +
  strlen(src)` in the ordinary case and `size + strlen(src)` when `dst` has no terminator. A
  return value `>= size` means the result was truncated, and `ret + 1` is the size that would
  have been enough.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definition in `mystrlcat.c`, below the marked line.

## Behaviour

The destination is shown as the exact bytes of a `size`-byte buffer.

| `dst` before (size 8) | `src` | Returns | `dst` after |
|---|---|---|---|
| `ab\0#####` | `"cd"` | `4` | `abcd\0###` |
| `\0#######` | `"cd"` | `2` | `cd\0#####` |
| `ab\0#####` | `""` | `2` | `ab\0#####` |
| `ab\0#####` | `"cdefg"` | `7` | `abcdefg\0` (an exact fit) |
| `ab\0#####` | `"cdefghij"` | `10` | `abcdefg\0` (truncated: `10 >= 8`) |
| `abcdef\0#` | `"xy"` | `8` | `abcdefx\0` (one character fitted) |
| `abcdefg\0` | `"xy"` | `9` | `abcdefg\0` (nothing fitted) |
| `caf\0####` | `"\xC3\xA9"` | `5` | `caf\xC3\xA9\0##` |
| `#` (size 1, no terminator) | `"ab"` | `3` | `#` (untouched) |
| `\0` (size 1) | `"ab"` | `2` | `\0` (nothing fits) |
| (size 0) | `"ab"` | `2` | nothing is read or written |
| `abcdefgh` (size 8, no terminator) | `"xy"` | `10` | `abcdefgh` (untouched) |

The last row is the contract's awkward case, and the three numbers in it are worth following:
`size` is 8, `strlen(src)` is 2, the return value is 10, and `10 >= 8` tells the caller it did
not fit. The function reported a problem without needing a special return value for it.

## Constraints

- **Do not call** `strcat`, `strncat`, `strcpy`, `stpcpy`, `strncpy`, `snprintf`, `strlcat` or
  `strlcpy`. The last two are not available under this course's feature macros anyway (SPEC
  section 10); the rest would do the job for you. `strlen` and `memcpy` are allowed.
- Never read `dst[i]` for `i >= size`, and never write one either.
- Correct for `size == 0`: `dst` may not be dereferenced at all.
- No allocation.
- One pass over `dst` (bounded by `size`) and one over `src`.

## What the harness checks

- **Fourteen fixed cases**, each with a destination of exactly `size` bytes whose contents are
  given byte by byte — including two that hold no terminator, one of `size` 1 and one of
  `size` 0. Both the return value and all `size` bytes are compared, and the fences are
  checked after every call.
- **The truncation protocol**: after an append that does not fit, `ret >= size` and
  `ret + 1 == 11` for the case in the table.
- **Oracle**: 3000 generated cases with a generated `size` from 0 to 24, comparing against
  `snprintf(ref, size, "%.*s%s", ...)` — which has the same contract and the same return
  value for a terminated destination — on both the bytes and the number.
- **Forbidden calls**: `mystrlcat.o` references none of the eight functions listed above.

## Verify

```bash
make check
```

Success ends with `s04-e02-my-strlcat: all checks passed` and `== check: OK`.

A failing fixed case shows the destination and the expected bytes:

```
FAIL main.c:23: truncated: returns 2 + 8
       got:  abcdefgh
       want: abcdefg\0
       first difference at index 7 of 8
```

Byte 7 holds a character where the terminator belongs: the copy used the whole buffer instead
of leaving a byte for the terminator. The oracle failure prints your buffer and `snprintf`'s
side by side, with both return values.

In the ASan stage, a `WRITE ... 0 bytes after` report means the terminator went to `dst[size]`,
and a `READ` report means the walk over `dst` was not bounded by `size`.

<details><summary>Hint</summary>

Measure both lengths first, in a way that cannot read past either buffer, and then decide.
The hard case is the one where the first measurement comes back equal to `size`.

</details>
