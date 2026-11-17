# s03-e01-my-strcpy-stpcpy `[WRITE]` (S)

## Goal

Write the two unbounded string copies of the C library yourself: `my_strcpy`, which behaves
exactly like `strcpy`, and `my_stpcpy`, which behaves exactly like POSIX `stpcpy`. Afterwards
you can say, byte for byte, what a string copy reads and writes, and you know why a copy
function should return the *end* of what it wrote rather than the start.

## Background

`strcpy` is the oldest string function there is, and the simplest: copy characters from the
source until the terminator has been copied. It takes no size. The caller promises that the
destination is big enough, and nothing checks the promise. That is the whole problem with
`strcpy`, and it is the reason the rest of this unit exists.

`strcpy` returns `dst`, the pointer you passed in. That return value tells you nothing you did
not know. `stpcpy` does the same copy and returns a pointer to the terminator it wrote, which
is exactly where the *next* piece of text goes. With it you can build a string from parts
without ever measuring or rescanning what you already wrote (the last check in this exercise
does that; S04 depends on it).

Read **The functions in this unit**, the `strcpy` and `stpcpy` cards, and **Worked example 1**
in `LESSON.md` before you start.

## The API

`mycopy.h` declares:

```c
char *my_strcpy(char *restrict dst, const char *restrict src);
char *my_stpcpy(char *restrict dst, const char *restrict src);
```

**`my_strcpy(dst, src)`** copies the string `src`, including its terminator, to `dst`.

- `src` is borrowed and is not modified. It is a string: it has a terminator.
- `dst` is the caller's buffer. The caller guarantees `strlen(src) + 1` bytes of room. You
  cannot check this and must not try.
- It writes exactly `strlen(src) + 1` bytes. The bytes of `dst` after the terminator keep
  whatever they held before.
- It returns `dst`.
- There is no error case and no failure return.

**`my_stpcpy(dst, src)`** makes the identical copy and returns a pointer to the terminator it
wrote: `dst + strlen(src)`. For an empty source that is `dst` itself.

`restrict` in the prototypes is the caller's promise that the two buffers do not overlap. You
do not have to handle overlap; the harness never passes overlapping buffers.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mycopy.c`, below the marked line.

## Behaviour

Destination buffers are shown before and after in byte notation; `#` is a byte that has not
been written.

| Call | `dst` before | `dst` after | Returns |
|---|---|---|---|
| `my_strcpy(dst, "hello")` | `########` | `hello\0##` | `dst` |
| `my_strcpy(dst, "")` | `########` | `\0#######` | `dst` |
| `my_strcpy(dst, "x")` | `########` | `x\0######` | `dst` |
| `my_strcpy(dst, "hello")`, `dst` has exactly 6 bytes | `######` | `hello\0` | `dst` |
| `my_strcpy(dst, "hi")`, over old contents | `goodbye\0` | `hi\0dbye\0` | `dst` |
| `my_strcpy(dst, "\xC3\xA9")` (bytes above 0x7F) | `########` | `\xC3\xA9\0#####` | `dst` |
| `my_stpcpy(dst, "hello")` | `########` | `hello\0##` | `dst + 5` |
| `my_stpcpy(dst, "")` | `########` | `\0#######` | `dst` |
| `my_stpcpy(dst, "x")` | `########` | `x\0######` | `dst + 1` |
| `e = my_stpcpy(dst, "usr"); my_stpcpy(e, "/")` | `########` | `usr/\0###` | `dst + 4` (second call) |

Note the fifth row: copying a short string over a longer one leaves the tail of the old string
in the buffer, after the new terminator. That is correct. A copy function does not clear the
destination.

## Constraints

- **Do not call** `strcpy`, `stpcpy`, `strncpy`, `stpncpy`, `strcat`, `strlen`, `memcpy` or
  `memmove`. The exercise is the loop itself, and a copy needs no length: the terminator tells
  you when to stop. The harness inspects your object file and fails if any of these is called.
- One pass over the source. Each source byte is read once.
- Read nothing after the source's terminator, and write nothing after the destination's.
- No allocation.

## What the harness checks

- **Five fixed strings** (ordinary, empty, one character, long, and one with bytes above
  0x7F), each copied three ways: into a buffer of *exactly* `strlen(src) + 1` bytes, into a
  buffer with four spare bytes that must still be `#` afterwards, and from a source buffer
  that ends at its terminator.
- **Return values**: `my_strcpy` returns `dst`; `my_stpcpy` returns `dst + strlen(src)`.
- **Chaining**: three `my_stpcpy` calls, each starting where the last one ended, build
  `"usr/bin"` in a 12-byte buffer.
- **Oracle**: 3000 generated strings of 0 to 40 characters, including bytes above 0x7F, are
  copied by your function and by libc's. All 48 bytes of the destination and the return value
  must agree.
- **Forbidden calls**: `mycopy.o` references none of the functions listed above.

## Verify

```bash
make check
```

Success ends with `s03-e01-my-strcpy-stpcpy: all checks passed` and `== check: OK`. The
ASan/UBSan stage prints nothing of its own.

A failed check looks like this:

```
FAIL main.c:23: my_strcpy copies the characters and the terminator
       got:  hello#
       want: hello\0
       first difference at index 5 of 6
```

Read `got` against `want` byte by byte: here byte 5 is still `#`, so the function copied the
five characters and never wrote the terminator. An oracle failure prints the generated source
string, your buffer and libc's.

In the ASan stage, a write past the end of the exact-size buffer stops the program with
`heap-buffer-overflow` and `WRITE of size 1`; the line "0 bytes after 6-byte region" means you
wrote the byte immediately after a 6-byte buffer. A `READ` report means you read past the
source's terminator.

<details><summary>Hint</summary>

Both functions are the same loop. Write the one whose return value is harder to produce
first; the other can then be one line.

</details>
