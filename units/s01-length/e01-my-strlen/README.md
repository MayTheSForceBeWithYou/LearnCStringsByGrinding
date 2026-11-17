# s01-e01-my-strlen `[WRITE]` (S)

## Goal

Write the C library's two measuring functions yourself: `my_strlen`, which behaves exactly
like `strlen`, and `my_strnlen`, which behaves exactly like POSIX `strnlen`. Afterwards you
know what each one costs, what each one trusts, and which of the two may be pointed at bytes
that are not a string — the difference that decides which one a line of real code should be
calling.

## Background

`strlen` is the most-called function in `<string.h>` and the one with the weakest contract: it
takes a pointer and nothing else, and it reads forward until it finds a zero byte. C17
7.24.6.3 says it "computes the length of the string pointed to by `s`", and that is the whole
specification. There is no bound, no error return, and no way for it to tell a string from a
character sequence. Hand it four characters in four bytes and it reads the fifth, the sixth,
and on until some unrelated byte happens to be zero.

`strnlen` is the repair, and it is not in ISO C: POSIX.1-2008 added it, and this course's
build declares it because `lib/dsalloc.h` defines `_POSIX_C_SOURCE 200809L`. It takes a
maximum and promises never to look past it. Its return value carries one extra piece of
information for free: a result equal to `maxlen` means *no terminator was found in those
bytes*, so the caller learns both the length and whether the bytes are a string, in one pass.

Writing both is worth the twenty minutes because the two loops differ in exactly one place —
the condition — and that one place is the difference between a function that trusts the
convention and a function that checks it. Everything else in this unit (fixed-width fields,
the `size_t` wrap, the unterminated heap copy) is a consequence of that choice.

Read **Foundations** and the `strlen` and `strnlen` cards in `LESSON.md` before you start.

## The API

`mylen.h` declares:

```c
size_t my_strlen(const char *s);
size_t my_strnlen(const char *s, size_t maxlen);
```

**`my_strlen(s)`** returns the number of bytes before the terminator of `s`.

- `s` is borrowed and is not modified. It must be a string: every byte from `s[0]` up to and
  including the first NUL is readable.
- It reads exactly `my_strlen(s) + 1` bytes and no byte after the terminator.
- It returns 0 for the empty string. There is no error return: a buffer with no terminator is
  not a valid argument, and this function cannot detect one.

**`my_strnlen(s, maxlen)`** returns the number of bytes before the first NUL among the first
`maxlen` bytes at `s`, or `maxlen` if there is no NUL among them.

- `s` need **not** be terminated: that is the point of the function. Only
  `min(maxlen, length + 1)` bytes have to be readable.
- `maxlen` may be 0, and then no byte of `s` is read at all and the result is 0. The harness
  calls it that way on a zero-byte buffer, so a read there is a report.
- It reads at most `maxlen` bytes and stops at the first NUL.
- A return value equal to `maxlen` means "no terminator was found"; a smaller one is a true
  length. There is no other error indication.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mylen.c`, below the marked line.

## Behaviour

Buffers are shown in byte notation: a character as itself, the terminator as `\0`.

| Call | The bytes at `s` | Returns | Why |
|---|---|---|---|
| `my_strlen(s)` | `hello\0` | `5` | five characters before the terminator |
| `my_strlen(s)` | `\0` | `0` | the empty string |
| `my_strlen(s)` | `ab\0cd\0` | `2` | it stops at the **first** NUL |
| `my_strlen(s)` | `caf\xC3\xA9\0` | `5` | bytes above 0x7F are ordinary characters |
| `my_strnlen(s, 6)` | `hello\0` | `5` | the terminator is within the six bytes |
| `my_strnlen(s, 5)` | `hello\0` | `5` | five bytes, no NUL among them: the answer is `maxlen` |
| `my_strnlen(s, 3)` | `hello\0` | `3` | likewise, three |
| `my_strnlen(s, 0)` | `hello\0` | `0` | no byte is read at all |
| `my_strnlen(s, 4)` | `A123` (no terminator) | `4` | a full field: `maxlen`, and `s[4]` is never read |
| `my_strnlen(s, 2)` | `A123` (no terminator) | `2` | |
| `my_strnlen(s, 6)` | `ab\0cd\0` | `2` | it stops at the first NUL, like `strlen` |
| `my_strnlen(s, 1)` | `ab\0cd\0` | `1` | the bound comes first |

The two rows that matter most are `my_strnlen(s, 5)` on `hello\0` and `my_strnlen(s, 4)` on
`A123`: both return `maxlen`, and in both cases the function must stop without looking at the
byte after the bound. A `my_strnlen` that measures first and clamps afterwards gives the same
*numbers* for every row of this table and is still wrong, because it read bytes it was told
not to.

## Constraints

- **Do not call** `strlen`, `strnlen`, `memchr`, `strchr`, `strcspn`, `strspn` or `strstr`.
  Those are the functions that would answer the question for you; the exercise is the loop and
  its stopping condition. The harness inspects `mylen.o` and fails if any of them is
  referenced, and your file is compiled with `-fno-builtin` so the compiler may not turn your
  loop into one of them either.
- One pass. Each byte of `s` is read at most once.
- `my_strlen` reads nothing after the terminator. `my_strnlen` reads nothing after the
  terminator **and** nothing at or after `s[maxlen]`.
- `my_strnlen` must be correct for `maxlen == 0` and for `maxlen == SIZE_MAX`: no expression
  that adds one to `maxlen`.
- No allocation.

## What the harness checks

- **Five fixed strings** (empty, one character, a word, a long sentence, one with bytes above
  0x7F), each held in a buffer of exactly `strlen(s) + 1` bytes, so that a read one byte past
  the terminator is a sanitizer report rather than a lucky pass. For each: `my_strlen`, and
  `my_strnlen` with `maxlen` one more than the length, equal to the length, and 0.
- **An embedded NUL** (`ab\0cd`): both functions must stop at the first one.
- **Three full fields** with no terminator anywhere (`A123`, three high bytes, one byte), each
  in a buffer of exactly its own width, with `my_strnlen` called at **every** `maxlen` from 0
  to the width. `my_strlen` is not called on these: it could not answer without reading past
  the end.
- **A zero-byte buffer**: `my_strnlen(s, 0)` must return 0 without touching it.
- **Oracle**: 3000 generated strings of 0 to 48 characters, including empty strings and bytes
  above 0x7F, each compared against libc's `strlen`, and `my_strnlen` compared against libc's
  `strnlen` at a generated `maxlen` from 0 to length + 2.
- **Forbidden calls**: `mylen.o` references none of the seven functions listed above.

## Verify

```bash
make check
```

Success ends with `s01-e01-my-strlen: all checks passed` and `== check: OK`, with the
forbidden-call check printing `forbid: none of [...] is called`.

A wrong answer shows up like this in the plain stage:

```
FAIL main.c:20: my_strnlen with maxlen equal to the length
```

Read the message against the table above: `maxlen` equal to the length means there is no
terminator inside the bound, so the answer is `maxlen` itself.

The more interesting failure is the one that **passes** the plain stage. A `my_strnlen`
written as "measure the whole string, then clamp the result" returns the right number every
time, and the ASan stage stops it at the read it was not allowed to make:

```
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 1 at 0x... thread T0
    #0 ... in my_strlen .../mylen.c:9
    #1 ... in my_strnlen .../mylen.c:17
    #2 ... in field_case .../main.c:31
```

`READ of size 1` with your own function in frame `#0`, reached from `field_case`, means a full
field was measured by walking past its last byte. That is the whole reason `strnlen` exists,
and it is why this harness keeps every field in a buffer of exactly its own width.

<details><summary>Hint</summary>

Write `my_strnlen` first. Its loop has two reasons to stop, and `my_strlen`'s has one of them.

</details>
