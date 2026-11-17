# s00-e02-count-and-terminate `[WRITE]` (S)

## Goal

Write the terminator convention out by hand: measure a string by index, find its end by
pointer, decide whether a buffer of known size is a string at all, and fill a buffer so that
it always is one. No library call is allowed anywhere in your file. Afterwards you know
exactly what `strlen` does and what it trusts, you can tell a *buffer* from a *string* in one
sentence, and you have written the `cap - 1` that the rest of the course is about.

## Background

C has no string type. It has a convention: a string is a sequence of bytes, the last of which
is zero, and a pointer to its first byte stands for the whole thing. Nothing records the
length, and nothing records the capacity of the storage it sits in. Every function in
`<string.h>` relies on that one zero byte being there; `strlen` reads until it finds one, and
if there is none it keeps reading memory it was never given.

So the two measurements of a `char` array are different questions with different answers:
`sizeof` (or a `cap` parameter) is how many bytes you may touch, and `strlen` is how many
bytes are in use. Between them sits the rule this whole course turns on: a string of length
`len` needs `len + 1` bytes, so it fits in `cap` bytes exactly when `len < cap`.

`buf_is_terminated` is the question that nothing in the library can answer for you. Given a
pointer and a capacity, it says whether the bytes are a string yet. Real code asks it after
reading a fixed-width record field, after a `strncpy`, and before handing a buffer to
anything that takes a `char *`. `buf_set` is the other side: a writer that leaves a buffer in
a state the readers can rely on, whatever it was asked to write.

Writing the first two by hand, once, in both of the shapes C programmers use (a subscript and
a moving pointer), is why this is the first `[WRITE]` of the course. You will reimplement
`strlen` properly in S01 e01 and compare it against libc; here the point is the convention,
not the function.

Read **Foundations** in `LESSON.md`, and the `sizeof` card, before you start.

## The API

`count.h` declares:

```c
size_t str_len(const char *s);
const char *str_end(const char *s);
int buf_is_terminated(const char *buf, size_t cap);
size_t buf_set(char *buf, size_t cap, char c, size_t n);
```

**`str_len(s)`** returns how many bytes come before the terminator of `s`. `s` is borrowed
and not modified; it must be a string. The function reads exactly `str_len(s) + 1` bytes and
no byte after the terminator. The empty string gives 0. There is no error return: a buffer
with no terminator is not a valid argument, and this function cannot detect one. **Write it
with a subscript** (`s[i]`) and a counter.

**`str_end(s)`** returns the address of the terminator itself, which is `&s[str_len(s)]`. It
is never NULL; for the empty string it is `s`. The requirements and the bytes read are those
of `str_len`. **Write it with one moving pointer**, no subscript and no counter. The
difference `str_end(s) - s` is the length, which is how you will measure strings in the rest
of the course when a pointer is what you have in hand.

**`buf_is_terminated(buf, cap)`** returns 1 if at least one of the `cap` bytes at `buf` is a
NUL, else 0. It reads at most `cap` bytes and stops at the first NUL. `cap` may be 0, and
then it reads no byte at all and returns 0: `buf` is not dereferenced. A return of 1 is the
promise that the buffer may be handed to a `<string.h>` function; 0 means those bytes are
data, and `strlen` on them is undefined behaviour.

**`buf_set(buf, cap, c, n)`** writes `min(n, cap - 1)` copies of `c` starting at `buf[0]`,
then one terminator, and returns the number of copies it wrote. So the result is always a
string, the function never writes `buf[cap]` or beyond, and a return value smaller than `n`
means the request did not fit. If `cap == 0` it writes nothing and returns 0. Bytes after the
terminator it writes keep their previous value: this function fills, it does not pad. `c` is
stored as it is, including a byte above 0x7F and including `'\0'` itself; `n` may be any
`size_t` value up to `SIZE_MAX`.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `count.c`, below the marked line.

## Behaviour

Buffers are shown in byte notation: a character as itself, the terminator as `\0`, and `#`
for a byte nothing has written.

| Call | Buffer before | Buffer after | Returns |
|---|---|---|---|
| `str_len("hello")` | | unchanged | `5` |
| `str_len("")` | | unchanged | `0` |
| `str_len(s)` where `s` is `a\0bc\0` | | unchanged | `1` |
| `str_end("hi")` | | unchanged | `s + 2`, pointing at `\0` |
| `str_end("")` | | unchanged | `s`, pointing at `\0` |
| `buf_is_terminated("hi\0junk", 7)` | | unchanged | `1` |
| `buf_is_terminated(p, 3)` where `p` is `abc` | | unchanged | `0` |
| `buf_is_terminated(p, 3)` where `p` is `\xFF\xE9\x80` | | unchanged | `0` |
| `buf_is_terminated(p, 0)` | | unchanged, not read | `0` |
| `buf_set(dst, 8, 'x', 3)` | `########` | `xxx\0####` | `3` |
| `buf_set(dst, 8, 'x', 7)` | `########` | `xxxxxxx\0` | `7` |
| `buf_set(dst, 8, 'x', 8)` | `########` | `xxxxxxx\0` | `7` |
| `buf_set(dst, 8, 'x', 0)` | `########` | `\0#######` | `0` |
| `buf_set(dst, 1, 'x', 5)` | `#` | `\0` | `0` |
| `buf_set(dst, 0, 'x', 4)` | (no bytes) | not touched | `0` |
| `buf_set(dst, 8, '\xE9', 2)` | `########` | `\xE9\xE9\0#####` | `2` |
| `buf_set(dst, 8, '\0', 3)` | `########` | `\0\0\0\0####` | `3` |
| `buf_set(dst, 8, 'y', SIZE_MAX)` | `########` | `yyyyyyy\0` | `7` |

The last three rows are the ones worth staring at. `c == '\0'` writes three NUL bytes and
then a fourth as the terminator, and the result is the empty string whose length is 0 even
though `buf_set` returned 3: the return value counts bytes written, not the length of the
string you can read back. `n == SIZE_MAX` must not be turned into `n + 1` anywhere, because
that is 0.

## Constraints

- **Do not call any library function** in `count.c`. The `FORBID` list is `strlen`, `strnlen`,
  `memset`, `memcpy`, `memmove`, `strcpy`, `stpcpy`, `strncpy`, `memchr` and `snprintf`: the
  functions that would do each of these jobs for you. The exercise is the convention itself,
  so it has to be bytes and a loop. Your file is compiled with `-fno-builtin`, so the
  compiler may not quietly turn your loop into a `memset` either, and the harness inspects
  `count.o` and fails if any of those names is referenced.
- `str_len` uses a subscript; `str_end` uses a moving pointer. Both read each byte once and
  stop at the terminator.
- Never read `buf[i]` for `i >= cap`, and never write `buf[i]` for `i >= cap`. With
  `cap == 0`, do not touch `buf` at all.
- `buf_set` must be correct for `n == SIZE_MAX`: no expression that computes `n + 1`.
- No allocation.

## What the harness checks

- **Five fixed strings** (empty, one character, a word, a long sentence, and one with bytes
  above 0x7F) for `str_len` and `str_end`, each held in a buffer of exactly `strlen(s) + 1`
  bytes, so that reading one byte past the terminator is a sanitizer report rather than a
  lucky pass. A string with an embedded NUL checks that both functions stop at the first one.
- **`buf_is_terminated`** on buffers of exactly `cap` bytes: a terminator at the first byte,
  in the middle, at the last byte, nowhere in three bytes, nowhere in ten, a buffer of three
  high bytes, and `cap == 0`. After each call the fences around the buffer are checked, so
  the function must not write anything.
- **`buf_set`** for the nine cases in the table above, each into a buffer of exactly `cap`
  bytes: the return value, all `cap` bytes of the buffer, and that nothing outside it was
  touched. A zero-capacity destination is a zero-byte buffer, so a single write into it is
  caught.
- **The four together**: after `buf_set(dst, 16, '=', 40)`, the result must be terminated
  according to `buf_is_terminated`, 15 characters long according to `str_len`, and
  `str_end(dst)` must be `dst + 15`.
- **Oracle**: 3000 generated strings of 0 to 48 characters, including bytes above 0x7F and
  empty strings, where `str_len` must equal libc's `strlen` and `str_end` must equal
  `s + strlen(s)`.
- **Forbidden calls**: `count.o` references none of the ten functions listed above.

## Verify

```bash
make check
```

Success ends with `s00-e02-count-and-terminate: all checks passed` and `== check: OK`, with
the forbidden-call check printing `forbid: none of [...] is called`.

The most common first failure is bounding `buf_set`'s loop by `cap` instead of `cap - 1`:

```
FAIL main.c:70: buf_set with n == cap truncates to cap - 1
FAIL main.c:71: buf_set with n == cap truncates to cap - 1
       got:  xxxxxxxx
       want: xxxxxxx\0
       first difference at index 7 of 8
FAIL main.c:72: buf_set wrote nothing outside the buffer
```

Read it as three separate statements. The return value was 8 where 7 was wanted (line 70).
The eighth byte of the buffer holds `x` where the terminator belongs (line 71), so the buffer
is not a string. And line 72 says the terminator went *outside* the buffer, at `buf[8]`,
which in the plain build is caught by the fence bytes around the destination.

In the ASan stage the same mistake stops the program at the write instead:

```
ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 1 at 0x... thread T0
    #0 ... in buf_set .../count.c:38
0x... is located 0 bytes after 8-byte region [0x...,0x...)
```

`WRITE of size 1` and `0 bytes after 8-byte region` together say: one byte, immediately after
an 8-byte buffer. Frame `#0` is the line in your file that did it. A `READ` report instead
means one of the measuring functions ran past the end of a source buffer.

<details><summary>Hint</summary>

Write `buf_set`'s loop so that the index it leaves behind is where the terminator goes, and
you will not need to decide afterwards whether to subtract one.

</details>
