# s02-r02-memset-memcmp-reps `[DRILL]` (XS)

## Skill

The four questions a program asks about raw bytes: clear this array, are these two ranges
equal, how much of a prefix do they share, and is this whole range one repeated byte. Each is
one line of `memset` or `memcmp` once the arithmetic is right, and the arithmetic is the drill.

You will write these in S06 (comparison), S12 (buffers from input), S16 (UTF-8 validation) and
in every `[FAULT]` exercise that has to prove a failed operation changed nothing. In real code
`zero_array` is the line that stops a reused record from leaking its predecessor's bytes, and
`common_prefix` is how a diff, an autocomplete and a radix tree all start.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `zero.c`, below the marked line.

## Functions

```c
void zero_array(void *base, size_t n, size_t size);
int ranges_equal(const void *a, const void *b, size_t n);
size_t common_prefix(const void *a, const void *b, size_t n);
int all_equal_to(const void *p, int c, size_t n);
```

| Function | Does | Examples |
|---|---|---|
| `zero_array` | sets every byte of `n` elements of `size` bytes to 0 | `zero_array(table, 3, sizeof *table)` clears three records, padding included |
| `ranges_equal` | 1 if the `n` bytes at `a` equal those at `b` | `ranges_equal("abc", "abd", 2)` is 1; `n == 0` is 1 |
| `common_prefix` | how many leading bytes they share, at most `n` | `common_prefix("abcdef", "abcxyz", 6)` is 3; equal ranges give `n` |
| `all_equal_to` | 1 if all `n` bytes at `p` equal `(unsigned char)c` | `all_equal_to("aaaa", 4, 'a')`… note the parameter order: `(p, c, n)` |

Four details the harness holds you to. `n == 0` is true for all three predicates — two empty
ranges are equal, share nothing, and are "all" of any byte — and writes nothing in
`zero_array`. `size == 0` or `n == 0` in `zero_array` writes nothing. `c` is converted to
`unsigned char`, so `all_equal_to(p, 0x1E9, n)` asks about the byte `0xE9`. And
`zero_array`'s job is **every byte**, including a struct's padding, which is why it takes
`n` and `size` separately rather than a total.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s02-mem-functions/drills/r02-memset-memcmp-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- `memset`, `memcmp` and `memchr` are the tools; `memccpy`, `strcpy`, `stpcpy`, `strncpy`,
  `strcmp`, `strncmp` and `strspn` are in `FORBID`. Three of the four functions should be one
  line.
- Pass sizes as `n * size` where that is what you mean, and write `sizeof *p` rather than
  `sizeof(Rec)` at the call sites in your own code: the first cannot go out of step with the
  type.
- `common_prefix` is the one that needs a loop. Keep it one pass and bound it by `n`.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness clears an array of three structs and checks every byte including the padding;
checks `zero_array` with `n == 0` and with `size == 0`; runs six pairs through
`ranges_equal`, six through `common_prefix` and seven through `all_equal_to`, including high
bytes, NUL bytes, a value above 255 and empty ranges; then compares `ranges_equal` and
`common_prefix` against libc on 2000 generated pairs over a two-letter alphabet, where shared
prefixes are common. It prints `s02-r02-memset-memcmp-reps: all checks passed`.

A `READ of size 1` in the ASan stage means a loop ran past `n`, which in `common_prefix` is
usually a condition in the wrong order: the bound has to be tested before the bytes.
