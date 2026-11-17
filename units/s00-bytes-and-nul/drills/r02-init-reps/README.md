# s00-r02-init-reps `[DRILL]` (XS)

## Skill

Putting bytes into a buffer you were given, without ever writing past its capacity and
without ever leaving something that is not a string. Four variants: make it empty, fill it
with a repeated byte, copy text into it, and ask whether the bytes in it are a string yet.

This is the other half of the course's basic motion (r01 reads; this writes), and it is the
shape of every exercise from S03 onwards: a `(buf, cap)` pair, a decision about what does not
fit, and a terminator you are responsible for. In real code it is the line between a log
message and a stack smash. The reason to drill it rather than learn it once is the `cap - 1`:
when the loop bound is automatic, the whole family of off-by-one buffer bugs stops being
something you can write.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `init.c`, below the marked line.

## Functions

```c
int init_empty(char *buf, size_t cap);
size_t init_repeat(char *buf, size_t cap, char c, size_t n);
size_t init_literal(char *buf, size_t cap, const char *lit);
int init_terminated(const char *buf, size_t cap);
```

In all four, `cap` is the total size of `buf` in bytes, terminator included, and it may be 0.
None of them writes `buf[cap]` or beyond; none pads, so bytes after the terminator keep
whatever they held.

| Function | Does | Returns | Example into 8 bytes |
|---|---|---|---|
| `init_empty` | writes one byte, `buf[0] = '\0'` | 0, or -1 and no write if `cap == 0` | `\0#######`, returns 0 |
| `init_repeat` | writes `min(n, cap - 1)` copies of `c`, then a terminator | the number of copies written | `init_repeat(buf, 8, '-', 99)` gives `-------\0`, returns 7 |
| `init_literal` | copies the characters of `lit` until it runs out of either, then a terminator | the number of characters copied | `init_literal(buf, 8, "abcdefgh")` gives `abcdefg\0`, returns 7 |
| `init_terminated` | looks for a NUL among the `cap` bytes | 1 if there is one, else 0 | `init_terminated("abc", 3)` is 0 |

Four details the harness holds you to. `cap == 0` means *no byte of `buf` may be touched*, by
any of them, including `init_terminated`, which must not read one either. `n` can be
`SIZE_MAX`, so nothing may compute `n + 1`. `c` is stored as it is, so a byte above 0x7F goes
in unchanged. And `init_literal` reads `lit` once, stopping at its terminator or at the byte
that does not fit, whichever comes first: it never reads the rest of a source that was cut.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s00-bytes-and-nul/drills/r02-init-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- No library call: `strlen`, `strnlen`, `memset`, `memcpy`, `memmove`, `strcpy`, `stpcpy`,
  `strncpy`, `snprintf` and `memchr` are in `FORBID`, and `init.c` is compiled with
  `-fno-builtin` so your loop stays a loop.
- Handle `cap == 0` first, in every function. Deciding it inside the loop condition works but
  reads as an accident; say it.
- An index (`buf[i]`) is the natural shape here. r01 is the pointer-walk drill.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs twenty-six cases: each function at a capacity that fits, one that fits
exactly, one that is one byte too small, `cap == 1` and `cap == 0`, plus `n == 0`,
`n == SIZE_MAX`, a byte above 0x7F, a multi-byte character cut in half, and buffers with the
terminator first, in the middle, at the end and nowhere. Every destination is a buffer of
exactly `cap` bytes with fences around it, and every source is a buffer of exactly its own
size. It finishes with the four used together, and prints
`s00-r02-init-reps: all checks passed`.

A failure in the plain build that names the byte at index `cap - 1` is the `cap - 1` bound; a
`check_guard` failure, or a `heap-buffer-overflow WRITE of size 1` with "0 bytes after" in
the ASan stage, is the terminator written at `buf[cap]`.
