# s03-e03-my-strlcpy `[WRITE]` (S)

## Goal

Write `my_strlcpy`, the bounded string copy that BSD added because `strncpy` was the wrong
tool: it always produces a string, it never pads, and its return value tells the caller
whether the copy was truncated and how big the buffer would have had to be. Afterwards you
have a copy function you can use for the rest of the course, and you know the contract well
enough to recognise it in `snprintf`.

## Background

By the 1990s the pattern `strncpy(dst, src, sizeof dst); dst[sizeof dst - 1] = '\0';` was
everywhere, and so were the bugs from forgetting its second half. OpenBSD's answer (1998) was
`strlcpy(dst, src, size)`:

- `size` is the **total size of the destination**, terminator included: you pass `sizeof dst`.
- The result is **always terminated** (unless `size` is 0, when there is no room for anything).
- It returns **`strlen(src)`**, the length the result would have had with unlimited room. So
  `ret >= size` means "truncated", and `ret + 1` is the size that would have been enough.

`strlcpy` is in the BSDs and macOS, and glibc gained it in version 2.38, but it is not in ISO C
and it is not declared under the feature-test macros this course compiles with. So you cannot
call it here. You can write it, in ten lines, and that is this exercise.

The standard library does offer the same contract under another name:
`snprintf(dst, size, "%s", src)` writes the same bytes and returns the same number (as an
`int`). The harness uses it as the oracle, which is also a demonstration: when you need a
bounded copy and have no `strlcpy`, you already have `snprintf`.

The price of the return value: `strlcpy` must read the whole source to measure it, even when
only three bytes fit. Copying a megabyte string into an 8-byte buffer reads a megabyte.

Read the `strlcpy` card and **Worked example 3** in `LESSON.md` first.

## The API

`mystrlcpy.h` declares:

```c
size_t my_strlcpy(char *restrict dst, const char *restrict src, size_t size);
```

**`my_strlcpy(dst, src, size)`** copies the string `src` into the `size`-byte buffer `dst`,
truncating if it must.

- If `size > 0`: it writes the first `min(strlen(src), size - 1)` characters of `src`, then
  one terminator. `dst` is always a string afterwards. There is no padding: bytes after the
  terminator are untouched.
- If `size == 0`: it writes nothing, and does not read `dst` either.
- It reads all of `src`, up to and including the terminator, in every case. `src` must be a
  string.
- It returns `strlen(src)`. There is no error return. The caller detects truncation with
  `if (my_strlcpy(dst, src, sizeof dst) >= sizeof dst)`.
- `src` is borrowed; `dst` is the caller's buffer; they do not overlap.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definition in `mystrlcpy.c`, below the marked line.

## Behaviour

`dst` is an 8-byte buffer holding `########` before every call.

| Call | `dst` after | Returns | Truncated? (`ret >= size`) |
|---|---|---|---|
| `my_strlcpy(dst, "hello", 8)` | `hello\0##` | 5 | no (5 < 8) |
| `my_strlcpy(dst, "abcdefg", 8)` | `abcdefg\0` | 7 | no (7 < 8): exact fit |
| `my_strlcpy(dst, "abcdefgh", 8)` | `abcdefg\0` | 8 | **yes** (8 >= 8): one too many |
| `my_strlcpy(dst, "abcdefghijkl", 8)` | `abcdefg\0` | 12 | **yes**; 13 bytes would have fit it |
| `my_strlcpy(dst, "", 8)` | `\0#######` | 0 | no |
| `my_strlcpy(dst, "hello", 1)` | `\0#######` | 5 | **yes**: room for the terminator only |
| `my_strlcpy(dst, "hello", 0)` | `########` | 5 | **yes**: nothing written at all |
| `my_strlcpy(dst, "a\xFFz", 3)` | `a\xFF\0#####` | 3 | **yes** |

Compare rows two and three. Seven characters fit in eight bytes; eight do not. The boundary
between "fits" and "truncated" is `strlen(src) == size - 1`, and the return value puts it in
one comparison.

## Constraints

- **Do not call** `strlen`, `strnlen`, `strcpy`, `strncpy`, `stpcpy`, `stpncpy`, `memcpy`,
  `memmove`, `snprintf` or `sprintf`. In particular, not "`strlen` then `memcpy`": that reads
  the source twice. The harness inspects your object file.
- One pass over `src`: every byte of it, terminator included, is read exactly once.
- Never write `dst[i]` for `i >= size`. With `size == 0`, never touch `dst`.
- Never pad.

## What the harness checks

- **The table above**, row by row: the return value, and all 8 bytes of the buffer.
- **The truncation idiom**: `"abc"` into a 4-byte array is reported as fitting, `"abcd"` as
  truncated, and the array still holds a valid string afterwards.
- **A full grid**: every source length 0 to 10 against every size 0 to 12, each in a buffer of
  *exactly* `size` bytes (a zero-byte buffer when `size` is 0), compared with
  `snprintf(ref, size, "%s", src)`: same bytes, same return value.
- **Oracle**: 2000 generated strings (with bytes above 0x7F) and generated sizes, the same
  comparison.
- **Forbidden calls**: `mystrlcpy.o` references none of the functions listed above.

## Verify

```bash
make check
```

Success ends with `s03-e03-my-strlcpy: all checks passed` and `== check: OK`.

A failed check, here from an implementation that stops counting when the buffer is full:

```
FAIL main.c:52: truncated: still returns the full source length
```

and, for the first grid pair that differs:

```
       my_strlcpy(dst, src, 0) is wrong (grid of every length 0..10 and every size 0..12); src: a\0
       yours:   returned 0
       want:    returned 1
```

(The buffers print as nothing because `size` is 0.) The return value is the source's length
whatever the size is.

In the ASan stage, `use-after-poison` with `WRITE of size 1`, located "0 bytes inside of 1-byte
region", comes from a `size == 0` case: the harness handed you a buffer of zero bytes and you
wrote a terminator into it. A `heap-buffer-overflow` WRITE "0 bytes after N-byte region" means
you wrote `dst[size]`.

<details><summary>Hint</summary>

Keep walking the source after the destination is full. Only the writing stops.

</details>
