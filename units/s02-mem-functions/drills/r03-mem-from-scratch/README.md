# s02-r03-mem-from-scratch `[DRILL]` (S)

## Skill

All five `mem` functions, from a blank file, in fifteen minutes. `memcpy`, `memmove`,
`memset`, `memcmp` and `memchr` — with the direction test, the `unsigned char` conversions and
every `n == 0` case right, checked against libc.

This is the "can you still do it" drill for the floor of the whole library. You wrote two of
them in e01 and three in e02; here they are all five together, repeatedly, until the five
contracts are one piece of knowledge rather than five. Capstone C1 asks for the whole of
`<string.h>` from a blank file, and every function in it is one of these five with a length
discovered on the way.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `scratch.c`, below the marked line.

## Functions

```c
void *s_memcpy(void *restrict dst, const void *restrict src, size_t n);
void *s_memmove(void *dst, const void *src, size_t n);
void *s_memset(void *s, int c, size_t n);
int s_memcmp(const void *a, const void *b, size_t n);
void *s_memchr(const void *s, int c, size_t n);
```

The library's prototypes with `s_` in front, and the library's contracts:

| Function | Contract, in one line | The part people forget |
|---|---|---|
| `s_memcpy` | `n` bytes, regions must not overlap, returns `dst` | the `restrict` is the caller's promise, not a check |
| `s_memmove` | `n` bytes, any overlap, returns `dst` | the direction depends on which pointer is lower |
| `s_memset` | `n` copies of `(unsigned char)c`, returns `s` | the conversion: `-1` is `0xFF` |
| `s_memcmp` | first `n` bytes as `unsigned char`; sign only | `0xFF` is greater than `0x01` |
| `s_memchr` | first byte equal to `(unsigned char)c`, or NULL | a NUL byte is found like any other |

`n == 0` is legal in all five: nothing is read or written, `s_memcmp` returns 0, `s_memchr`
returns NULL, and the two copies still return `dst`.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s02-mem-functions/drills/r03-mem-from-scratch`.
- No lookups during a rep: not the lesson, not `man`, not e01 or e02, not an earlier rep. If
  you get stuck, stop the clock, read, and start a new rep another day.
- No library call: all five libc functions are in `FORBID`, along with `memccpy`, `strcpy`,
  `stpcpy`, `strncpy` and `strlen`, and `scratch.c` is compiled with `-fno-builtin` so your
  loops stay loops.
- Work through `unsigned char *`, and convert `c` once, before the loop.
- `s_memmove` chooses a direction; it does not allocate and does not use an array of `n`
  bytes.
- No allocation anywhere.

## Target

15 minutes per rep for all five. At least 3 reps, on different days. If you are over twenty
minutes, the thing to practise next is not these five functions but the `size_t` loop shapes
in S01 r01 and r02.

## Verify

```bash
make check
```

The harness checks `s_memcpy` on three fixed buffers including `n == 0` and bytes above 0x7F;
sweeps `s_memmove` over every overlap from −16 to +16 for `n` of 1 and 12, comparing against
libc's `memmove`; checks `s_memset` for a prefix, for `0x141` and for `n == 0`; checks
`s_memcmp` on `0x7F` against `0x80` in both orders and with `n == 0`; checks `s_memchr` on six
cases including a NUL byte, a byte after it, a bound that excludes the match and `n == 0`;
then runs all five against libc on 2000 generated buffers. It prints
`s02-r03-mem-from-scratch: all checks passed`.

The two failures worth recognising: a sweep mismatch means `s_memmove` copies in one
direction only, and a `FAIL` on the `0x7F`/`0x80` pair means `s_memcmp` compares as `char`.
