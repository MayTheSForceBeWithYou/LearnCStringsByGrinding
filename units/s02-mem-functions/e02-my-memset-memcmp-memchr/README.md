# s02-e02-my-memset-memcmp-memchr `[WRITE]` (S)

## Goal

Write the other three `mem` functions: `my_memset`, which fills, `my_memcmp`, which compares,
and `my_memchr`, which searches. All three take a byte count and none of them knows what a
string is. Afterwards you know why two of them take their byte as an `int`, what "compares as
`unsigned char`" buys you, and why `memcmp`'s return value may only be looked at through its
sign.

## Background

These three are the primitives under half of `<string.h>`. `strcmp` is `memcmp` with the
length discovered as it goes; `strchr` is `memchr` with the same; a bounded `strnlen` is
`memchr` for a zero byte. Writing them makes three details stick, and all three are places
where careful C differs from C that happens to work.

**The byte is an `int`.** `memset(p, c, n)` and `memchr(s, c, n)` take `c` as an `int` and
convert it to `unsigned char` before use (C17 7.24.6.1, 7.24.5.1). The `int` is historical —
before prototypes, a `char` argument was promoted to `int` anyway — and the conversion is what
makes `memset(p, -1, n)` fill with `0xFF` and `memset(p, 0x141, n)` fill with `'A'`. A caller
who passes a value outside 0..255 gets the low eight bits, silently and by specification.

**The comparison is unsigned.** C17 7.24.4 says the bytes are "interpreted as `unsigned
char`", so `0xFF` is greater than `0x01` and `0x80` is greater than `0x7F` — on every
platform, whatever plain `char` is. Write the comparison with `char` and your function
disagrees with libc for every byte above 0x7F on a machine where `char` is signed, which is
both course platforms. That is S06 e03's defect, one unit early.

**Only the sign is specified.** `memcmp` returns "an integer greater than, equal to, or less
than zero"; it does not return -1, 1, or the difference of the bytes. glibc has returned all
three over the years. Code that tests `memcmp(a, b, n) == 1` is broken and will pass its
tests, which is why this harness compares `sign_of()` on both sides and never a value.

Read **Foundations** and the three cards in `LESSON.md` before you start.

## The API

`mymem.h` declares:

```c
void *my_memset(void *s, int c, size_t n);
int my_memcmp(const void *a, const void *b, size_t n);
void *my_memchr(const void *s, int c, size_t n);
```

**`my_memset(s, c, n)`** writes `n` copies of `(unsigned char)c` at `s` and returns `s`. It
reads nothing. `n` bytes must be writable; `n` may be 0, and then nothing is written.

**`my_memcmp(a, b, n)`** compares the first `n` bytes at `a` and `b` as `unsigned char` and
returns a negative value, zero, or a positive value as the first differing byte is smaller,
absent, or larger in `a`. Only the sign is specified: your function may return -1 and 1, or
the byte difference, or anything with the right sign. `n` bytes must be readable at each; it
may stop at the first difference; `n == 0` returns 0.

**`my_memchr(s, c, n)`** returns a pointer to the first byte equal to `(unsigned char)c` among
the first `n` bytes at `s`, or NULL if there is none. It reads at most `n` bytes and stops at
the byte it finds. A NUL byte is found like any other. `n == 0` returns NULL and reads
nothing.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mymem.c`, below the marked line.

## Behaviour

`dst` is an 8-byte buffer holding `########` before each `my_memset` call.

| Call | Result | Why |
|---|---|---|
| `my_memset(dst, 'x', 3)` | `xxx#####` | three bytes, the rest untouched |
| `my_memset(dst, 0, 4)` | `\0\0\0\0####` | `c == 0` is an ordinary byte value |
| `my_memset(dst, 0xE9, 2)` | `\xE9\xE9######` | a byte above 0x7F |
| `my_memset(dst, 0x141, 4)` | `AAAA####` | converted to `unsigned char`: 0x41 |
| `my_memset(dst, -1, 2)` | `\xFF\xFF######` | and -1 becomes 0xFF |
| `my_memset(dst, 'x', 0)` | `########` | `n == 0` writes nothing |
| `my_memcmp("abc", "abc", 3)` | sign `0` | equal |
| `my_memcmp("abc", "abd", 3)` | sign `-1` | `'c' < 'd'` |
| `my_memcmp("abc", "abd", 2)` | sign `0` | the difference is past `n` |
| `my_memcmp("\x7F", "\x80", 1)` | sign `-1` | **as `unsigned char`**: 127 < 128 |
| `my_memcmp("\xFF", "\x01", 1)` | sign `1` | 255 > 1, not negative |
| `my_memcmp("a\0b", "a\0c", 3)` | sign `-1` | a NUL byte does not end the comparison |
| `my_memcmp(a, b, 0)` | `0` | no bytes compared |
| `my_memchr("hello", 'l', 5)` | `s + 2` | the **first** match |
| `my_memchr("hello", 'z', 5)` | `NULL` | not found |
| `my_memchr("hello", 'l', 2)` | `NULL` | `n` comes first |
| `my_memchr("ab\0cd", 0, 5)` | `s + 2` | a NUL byte is found like any other |
| `my_memchr("ab\0cd", 'd', 5)` | `s + 4` | and the search continues past it |
| `my_memchr(s, 'x', 0)` | `NULL` | nothing is read |

## Constraints

- **Do not call** `memset`, `memcmp`, `memchr`, `memcpy`, `memmove`, `strlen`, `strnlen`,
  `strchr`, `strcmp` or `strncmp`. The harness inspects `mymem.o`, and your file is compiled
  with `-fno-builtin` so a loop stays a loop.
- Work through `unsigned char *`: it is the type that may alias any object's bytes (C17
  6.5p7), and it is what makes the comparison and the conversion correct without any thought
  about the signedness of `char`.
- Convert `c` to `unsigned char` **once**, before the loop, in both functions that take it.
- `my_memcmp` and `my_memchr` may stop early; neither may read a byte at or past index `n`.
- No allocation.

## What the harness checks

- **`my_memset`** on nine cases: the whole buffer, a prefix, `n == 0`, `c == 0`, a high byte,
  `0x141`, `-1`, a one-byte buffer, and a zero-byte buffer. Every destination is exactly its
  stated size and all of its bytes are compared, so a write past `n` and an unwritten byte are
  both visible.
- **`my_memcmp`** on nine pairs, each side held in a buffer of exactly `n` bytes: equal,
  differing in either direction, a difference past `n`, `n == 0`, `0x7F` against `0x80`,
  `0xFF` against `0x01`, an embedded NUL, and a prefix. Each pair is checked for the right
  sign, for the opposite sign when the arguments are swapped, and for agreeing in sign with
  libc's `memcmp`.
- **`my_memchr`** on twelve cases: a match at the first, middle and last byte, no match, a
  bound that excludes the match, a NUL byte, a byte after a NUL, a high byte, a value above
  255, `n == 0`, and a zero-byte buffer.
- **The three together**: two buffers filled identically compare equal and the fill byte is
  found at index 0; after changing one byte, the comparison's sign and the search's result
  both reflect it.
- **Oracle**: 3000 generated buffers over a two-letter alphabet (so that equal prefixes are
  common), comparing your three against libc's — sign only for `memcmp`, pointer identity for
  `memchr`, all bytes for `memset`.
- **Forbidden calls**: `mymem.o` references none of the ten functions listed above.

## Verify

```bash
make check
```

Success ends with `s02-e02-my-memset-memcmp-memchr: all checks passed` and `== check: OK`.

The failure that catches most first attempts is comparing as `char`:

```
FAIL main.c:27: 0x7F is smaller than 0x80: bytes compare as unsigned char
FAIL main.c:28: and the reverse comparison has the opposite sign
FAIL main.c:29: and it agrees with memcmp
FAIL main.c:27: 0xFF is larger than 0x01, not smaller
```

On this platform plain `char` is signed, so `0x80` is -128 and `0xFF` is -1, and a comparison
written with `char` gets both of those backwards. The three lines are the same case checked
three ways: the sign, the sign when the arguments are swapped, and agreement with libc.

In the ASan stage, a `READ of size 1` with `0 bytes after` means a loop went one byte past
`n` — usually `i <= n` instead of `i < n`, or a `my_memchr` that tests the byte after the loop
has ended.

<details><summary>Hint</summary>

All three start the same way: one `unsigned char *` for each buffer, and for two of them one
`unsigned char` holding the converted value.

</details>
