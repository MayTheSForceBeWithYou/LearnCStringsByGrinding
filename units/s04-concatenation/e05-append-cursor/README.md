# s04-e05-append-cursor `[WRITE]` (M)

## Goal

Build the appender this unit has been leading to: a small struct that remembers a buffer, its
capacity, how much is used and whether anything has been dropped, so that appending is a call
that cannot overflow, cannot forget to terminate, and does not rescan. Afterwards you have the
shape you will reuse for the rest of the course, and you have written a `vsnprintf` wrapper,
which is the one piece of variadic plumbing every C program ends up needing.

## Background

Four appends into a fixed buffer, done with the library directly, is four chances to get the
room arithmetic wrong and four return values to check:

```c
if (strlcat(buf, a, sizeof buf) >= sizeof buf) ...        /* or snprintf, or strncat */
if (strlcat(buf, b, sizeof buf) >= sizeof buf) ...
```

Every call re-measures what is already there (the quadratic shape of e03), and in practice
nobody checks all four return values — they check the last one, or none.

A **cursor** fixes both. It keeps `len` beside the buffer, so an append writes at
`buf + len` with no walk; and it keeps a sticky `truncated` flag, so the caller checks once,
at the end, no matter how many appends happened. That is the trade: a little state, in
exchange for appends that are O(what you append) and an error check that cannot be forgotten
in the middle.

The sticky flag deserves a sentence of design. Once anything has been dropped, the buffer no
longer holds the string the caller asked for, and a later append that fits does not change
that. If the flag were cleared by a successful append, a caller checking at the end would be
told everything was fine. So: set on any loss, never cleared except by `cur_init`.

`cur_printf` is the part that is new. `printf`-style formatting into a cursor needs
`vsnprintf`, which takes a `va_list` instead of `...`; the wrapper collects the arguments with
`va_start`, passes the list along, and ends with `va_end`. `vsnprintf` returns the length the
complete output *would* have needed, which is what tells the cursor whether anything was
dropped — the S03 `snprintf` return-value contract, used for real.

Read **Foundations: an appender keeps its place** in `LESSON.md` before you start.

## The API

`cursor.h` declares:

```c
typedef struct {
    char *buf;
    size_t cap;
    size_t len;
    int truncated;
} Cursor;

void cur_init(Cursor *c, char *buf, size_t cap);
void cur_puts(Cursor *c, const char *s);
void cur_putc(Cursor *c, char ch);
void cur_printf(Cursor *c, const char *fmt, ...);
int cur_truncated(const Cursor *c);
size_t cur_len(const Cursor *c);
```

The struct's fields are fixed and the harness reads none of them directly; `cur_len` and
`cur_truncated` are how it asks.

**`cur_init(c, buf, cap)`** prepares a cursor over `buf`, which has `cap` bytes. If `cap > 0`
the buffer becomes the empty string at once, so it is a string even if nothing is appended.
`cap` may be 0, and then nothing is ever written and every append is a truncation. It clears
the truncated flag.

**`cur_puts(c, s)`** appends `s`, or as much of it as fits, terminates, and sets the flag if
any character was dropped.

**`cur_putc(c, ch)`** appends one character if it fits, terminates, and sets the flag if it
does not.

**`cur_printf(c, fmt, ...)`** formats and appends, or appends as much as fits, and sets the
flag if anything was dropped or if `vsnprintf` returned a negative value.

**`cur_truncated(c)`** is 1 once anything has ever been dropped. **`cur_len(c)`** is the
number of characters in the buffer — what `strlen` would say, without the walk.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `cursor.c`, below the marked line.

## Behaviour

| Sequence | Buffer afterwards | `cur_len` | `cur_truncated` |
|---|---|---|---|
| `cur_init` over 16 bytes | `` | 0 | 0 |
| then `puts "ab"`, `putc '/'`, `puts "cd"` | `ab/cd` | 5 | 0 |
| `cur_init` over 4, `puts "abc"` | `abc` | 3 | 0 (an exact fit) |
| then `putc 'd'` | `abc` | 3 | 1 |
| `cur_init` over 8, `puts "abcdefghij"` | `abcdefg` | 7 | 1 |
| then `puts ""`, `putc 'x'` | `abcdefg` | 7 | 1 (sticky) |
| `cur_init` over 16, `printf "%s=%d", "n", 42` | `n=42` | 4 | 0 |
| then `printf " %s", "ok"` | `n=42 ok` | 7 | 0 |
| then `printf " %s", "0123456789"` | `n=42 ok 0123456` | 15 | 1 |
| `cur_init` over 1, any append | `` | 0 | 1 |
| `cur_init` over 0, any append | (nothing written) | 0 | 1 |

Two rows to look at twice. An **exact fit is not a truncation**: filling the buffer completely
is success. And with `cap == 1` the buffer holds only a terminator, so every append is a loss
— while with `cap == 0` there is not even a terminator to write, and `cur_init` must not touch
the buffer at all.

## Constraints

- **Do not call** `strcat`, `strncat`, `strlcat`, `strlcpy`, `strcpy`, `stpcpy` or `strncpy`.
  `strlen`, `memcpy` and `vsnprintf` are the tools. The harness inspects `cursor.o`.
- No allocation: the cursor never owns the buffer.
- No rescanning: no `strlen` of the destination anywhere. The cursor knows its length.
- Never write `buf[cap]` or beyond, and never read or write anything when `cap == 0`.
- After every call with `cap > 0` the buffer is a string.
- `cur_printf` uses `vsnprintf` and must handle both of its failure signals: a negative return
  and a return `>= ` the room it was given.

## What the harness checks

- **The ordinary chain**: three appends, the resulting string, `cur_len` against `strlen`, and
  the fences.
- **An exact fit** followed by one character too many: the flag is clear after the first and
  set after the second, and the length does not change.
- **Stickiness**: a truncation, then `cur_init` (which clears it), then another truncation,
  then an empty append and a one-character append, with the flag still set.
- **`cur_printf`**: two formats that fit, one that does not, the resulting string, the length
  (`cap - 1` when full), and the flag.
- **`cap == 1` and `cap == 0`**: every append is a truncation, the length stays 0, and for
  `cap == 0` the zero-byte buffer is never touched.
- **An empty append and bytes above 0x7F**.
- **A dirty buffer**: `cur_init` over a buffer that already holds text must make it the empty
  string, and a later append must leave the bytes past the new terminator alone.
- Every buffer is exactly `cap` bytes with fences around it, so any write at `buf[cap]` is a
  fence failure in the plain build and a `heap-buffer-overflow` under ASan.

## Verify

```bash
make check
```

Success ends with `s04-e05-append-cursor: all checks passed` and `== check: OK`.

A failing check names the property rather than the bytes:

```
FAIL main.c:47: an exact fit is not a truncation
```

That one means the room test is off by one in the generous direction's opposite: filling the
buffer exactly set the flag. The usual cause is testing `len + add >= cap` where the
terminator has already been accounted for.

In the ASan stage, a `WRITE ... 0 bytes after` report means a terminator went to `buf[cap]` —
most often from `cur_printf`, where the size passed to `vsnprintf` must be `cap - len` and not
`cap`.

<details><summary>Hint</summary>

Write one small function that answers "how many characters fit here now", and let the other
three use it.

</details>
