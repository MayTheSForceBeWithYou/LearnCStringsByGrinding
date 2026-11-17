# s03-e02-my-strncpy `[WRITE]` (S)

## Goal

Write `my_strncpy` and `my_stpncpy` with exactly the semantics of the library functions,
including the two parts that surprise people: the padding, and the missing terminator. After
this exercise you will never again read `strncpy(dst, src, sizeof dst)` as "a safe `strcpy`",
because you will have written the loop that makes it unsafe.

## Background

`strncpy` is the most misunderstood function in `<string.h>`. Its name suggests "`strcpy` with
a bound". Its real job is narrower: **fill a fixed-width field**. It always writes exactly `n`
bytes. If the source is shorter than the field, the rest of the field is filled with NUL
bytes. If the source is as long as the field or longer, the field is filled with characters
and **no terminator is written**, because a full fixed-width field does not have one.

That design comes from early Unix, where a directory entry held a file name in 14 bytes: a
short name was NUL-padded, a 14-character name used all 14 bytes. The same layout survives in
`utmp` records, tar headers and many binary file formats, and for those `strncpy` is still the
right tool.

The defect it causes everywhere else is this: code copies with `strncpy(dst, src, sizeof dst)`
and then treats `dst` as a string. It is one only when the source happened to be short enough.
`s03-e05` is that bug; here you write the function that produces it.

`stpncpy` (POSIX) does the same job and returns something useful: a pointer to the first NUL
it wrote, or `dst + n` if it wrote none. Comparing the return value with `dst + n` is how a
caller of `stpncpy` learns that the field is full.

Read the `strncpy` and `stpncpy` cards and **Worked example 2** in `LESSON.md` first.

## The API

`mystrncpy.h` declares:

```c
char *my_strncpy(char *restrict dst, const char *restrict src, size_t n);
char *my_stpncpy(char *restrict dst, const char *restrict src, size_t n);
```

**`my_strncpy(dst, src, n)`** fills the `n` bytes at `dst` from the string `src`.

- It writes **exactly `n` bytes, always**: first the characters of `src`, stopping after `n`
  characters or at `src`'s terminator, whichever comes first; then NUL bytes until `n` bytes
  have been written.
- If `src` has `n` or more characters, no NUL is written. `dst` is then not a string.
- It reads at most `n` bytes of `src`, and never reads past `src`'s terminator. `src` need not
  be terminated if it has at least `n` readable bytes.
- `n == 0` is valid: nothing is read, nothing is written.
- It returns `dst`. It reports neither truncation nor anything else.

**`my_stpncpy(dst, src, n)`** writes the same `n` bytes and returns:

- `dst + strlen(src)`, a pointer to the first NUL it wrote, when `src` is shorter than `n`;
- `dst + n` when it wrote no NUL (so `ret == dst + n` means "the field is full").

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mystrncpy.c`, below the marked line.

## Behaviour

`dst` is an 8-byte buffer holding `########` before every call.

| Call | `dst` after | Returns | Is `dst` a string? |
|---|---|---|---|
| `my_strncpy(dst, "hello", 8)` | `hello\0\0\0` | `dst` | yes (padded) |
| `my_strncpy(dst, "hello", 6)` | `hello\0##` | `dst` | yes (the one NUL is the padding) |
| `my_strncpy(dst, "hello", 5)` | `hello###` | `dst` | **no** |
| `my_strncpy(dst, "hello", 3)` | `hel#####` | `dst` | **no** |
| `my_strncpy(dst, "", 4)` | `\0\0\0\0####` | `dst` | yes (empty) |
| `my_strncpy(dst, "hello", 0)` | `########` | `dst` | nothing was written |
| `my_strncpy(dst, "abcdefgh", 8)` | `abcdefgh` | `dst` | **no** (full field) |
| `my_strncpy(dst, "a\xFFz", 8)` | `a\xFFz\0\0\0\0\0` | `dst` | yes |
| `my_stpncpy(dst, "hey", 8)` | `hey\0\0\0\0\0` | `dst + 3` | yes |
| `my_stpncpy(dst, "hello", 5)` | `hello###` | `dst + 5` (`== dst + n`) | **no** |
| `my_stpncpy(dst, "", 4)` | `\0\0\0\0####` | `dst` | yes (empty) |
| `my_stpncpy(dst, "abc", 0)` | `########` | `dst` | nothing was written |

The third row is the one to stare at. Five characters, `n == 5`, five bytes written, and the
byte after them is whatever was there before.

## Constraints

- **Do not call** `strncpy`, `stpncpy`, `strcpy`, `stpcpy`, `strlen`, `strnlen`, `memcpy`,
  `memmove` or `memset`. You are writing the loop, including the padding. The harness inspects
  your object file.
- Match libc exactly, including the behaviour you would not have designed. Do not "fix"
  `strncpy` by adding a terminator: a caller filling a 14-byte field with a 14-character name
  needs all 14 bytes.
- Never read `src[i]` for `i >= n`, and never read past `src`'s terminator.
- Never write `dst[i]` for `i >= n`.

## What the harness checks

- **The table above**, row by row, comparing all 8 bytes of the buffer each time, so both a
  missing pad byte and an extra terminator are visible.
- **A full grid**: every source length from 0 to 10 against every `n` from 0 to 12 (143
  pairs), each compared with libc's `strncpy` and `stpncpy`. The destination has *exactly* `n`
  bytes. The source buffer holds *exactly* the bytes the function may read: the string and its
  terminator when it is shorter than `n`, otherwise its first `n` characters with no
  terminator after them.
- **Oracle**: 2000 generated strings (with bytes above 0x7F) and generated `n`.
- **Return values** of both functions in every case.
- **Forbidden calls**: `mystrncpy.o` references none of the functions listed above.

## Verify

```bash
make check
```

Success ends with `s03-e02-my-strncpy: all checks passed` and `== check: OK`.

A failed check, here from an implementation that "helpfully" always terminates:

```
FAIL main.c:64: source of exactly n characters: n bytes, no terminator
       got:  hell\0###
       want: hello###
       first difference at index 4 of 8
```

The fifth character was replaced by a NUL: that function loses data that the real `strncpy`
keeps. A grid or oracle failure prints the source, `n`, your bytes and libc's bytes once, for
the first pair that differs.

In the ASan stage, a report with `READ of size 1` means you read `src[n]`: you tested for the
terminator before testing the count. `WRITE of size 1` means you wrote `dst[n]`. For `n > 0`
the report is a `heap-buffer-overflow` located "0 bytes after n-byte region". For `n == 0` the
harness hands you buffers of zero bytes, and touching one is reported as `use-after-poison`,
"0 bytes inside of 1-byte region": the same mistake, a different label.

<details><summary>Hint</summary>

There are two loops, and the pointer `my_stpncpy` returns is the position between them.

</details>
