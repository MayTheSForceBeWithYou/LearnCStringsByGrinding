# s02-e07-swap-reverse-rotate `[WRITE]` (S)

## Goal

Three array operations for elements of **any** size, built only out of `memcpy` and
`memmove`: exchange two elements, reverse an array, and rotate it left by `k` places. The
scratch space is a fixed 32-byte buffer, so an element may be larger than anything you can
hold at once. Afterwards you can write generic array code the way the C library does it — a
base pointer, a count and an element size — and you know the three-reversal trick, which is
the neatest algorithm in this course.

## Background

C has no generic types, so the library's generic functions take the element size as a
parameter: `qsort(base, n, size, cmp)`, `bsearch(key, base, n, size, cmp)`. An element is
`size` bytes at `base + i * size`, and the only operations available on it are the `mem`
functions, because nothing else knows what the bytes mean. Everything in this exercise is
that pattern.

**Swapping needs scratch space**, and the scratch space is fixed while the element size is
not. A 32-byte buffer and a 100-byte element is not a problem — exchange the first 32 bytes,
then the next 32, then the next 32, then the last 4 — and it is a problem worth having solved
once, because this is exactly what `qsort` does internally for large elements.

**Reversing is swapping, from both ends inwards.** The loop has to stop in the middle, and
the middle element of an odd-length array is swapped with itself or not at all.

**Rotating left by `k`** moves the element at index `k` to index 0 and the first `k` elements
to the end, in order. The obvious implementations need either a second array of `n` elements
or `n` separate moves; the three-reversal trick needs neither:

> reverse the first part, reverse the second part, reverse the whole thing.

It is worth knowing not only because it is elegant but because it is the standard way to
rotate in place with O(1) extra space, and because it is a one-line proof that reversal
composes: `reverse(reverse(A) reverse(B))` is `B A`. The arithmetic around it — `k` of 0, `k`
of `n`, `k` larger than `n`, `n` of 0 — is where the exercise is.

Read **Foundations** and the `memcpy` and `memmove` cards in `LESSON.md` before you start.

## The API

`memops.h` declares:

```c
void mem_swap(void *a, void *b, size_t size);
void mem_reverse(void *base, size_t n, size_t size);
void mem_rotate_left(void *base, size_t n, size_t size, size_t k);
```

and one constant, `MEMOPS_CHUNK` (32), the size of the scratch buffer you may use.

**`mem_swap(a, b, size)`** exchanges the `size` bytes at `a` with the `size` bytes at `b`.
The two regions do not overlap, except that `a == b` is allowed and must do nothing visible.
`size` may be 0. `size` may also be larger than `MEMOPS_CHUNK`, so an element has to be
exchanged in pieces.

**`mem_reverse(base, n, size)`** reverses the order of the `n` elements of `size` bytes each at
`base`. `n` of 0 or 1 changes nothing. Elements larger than the scratch buffer must work.

**`mem_rotate_left(base, n, size, k)`** rotates the `n` elements left by `k` places: the
element at index `k` ends up at index 0, and the first `k` elements move to the end keeping
their order. `k` may be anything: 0, `n`, or much larger than `n` — rotating by `k` is
rotating by `k % n`. `n` of 0 changes nothing (and `k % n` is not something you may compute
then).

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `memops.c`, below the marked line.

## Behaviour

Arrays are written one letter per element (so `abcde` is five elements, each `size` bytes
holding one repeated letter).

| Call | Array before | Array after |
|---|---|---|
| `mem_swap(a, b, 4)` | `aaaa` and `bbbb` | `bbbb` and `aaaa` |
| `mem_swap(a, a, 4)` | `bbbb` | `bbbb` |
| `mem_swap(a, b, 0)` | anything | unchanged |
| `mem_swap(a, b, 100)` | 100 `A`s and 100 `B`s | exchanged, in pieces of at most 32 |
| `mem_reverse(base, 5, 1)` | `abcde` | `edcba` |
| `mem_reverse(base, 4, 1)` | `abcd` | `dcba` |
| `mem_reverse(base, 1, 1)` | `a` | `a` |
| `mem_reverse(base, 0, 1)` | (empty) | unchanged |
| `mem_reverse(base, 2, 33)` | `ab`, 33 bytes each | `ba` |
| `mem_rotate_left(base, 5, 1, 0)` | `abcde` | `abcde` |
| `mem_rotate_left(base, 5, 1, 1)` | `abcde` | `bcdea` |
| `mem_rotate_left(base, 5, 1, 2)` | `abcde` | `cdeab` |
| `mem_rotate_left(base, 5, 1, 4)` | `abcde` | `eabcd` |
| `mem_rotate_left(base, 5, 1, 5)` | `abcde` | `abcde` (`k == n`) |
| `mem_rotate_left(base, 5, 1, 7)` | `abcde` | `cdeab` (`7 % 5` is 2) |
| `mem_rotate_left(base, 6, 1, 3)` | `abcdef` | `defabc` |
| `mem_rotate_left(base, 1, 1, 3)` | `a` | `a` |
| `mem_rotate_left(base, 0, 1, 3)` | (empty) | unchanged |
| `mem_rotate_left(base, 3, 40, 2)` | `abc`, 40 bytes each | `cab` |

## Constraints

- **Do not call** `memccpy`, `strcpy`, `stpcpy`, `strncpy` or `strcat`. `memcpy`, `memmove`
  and `memset` are the tools here and are not forbidden — this exercise is about using them,
  not reimplementing them.
- **No allocation.** The only scratch space is a buffer of `MEMOPS_CHUNK` bytes on the stack,
  and an element may be larger than it. No variable-length arrays either.
- Never read or write outside the array: `base + n * size` is one past the end.
- `mem_rotate_left` must be correct for `n == 0` without computing `k % 0`.
- Each of the three may call the others; that is the intended shape.

## What the harness checks

- **`mem_swap`**: a four-byte exchange, `a == b`, `size == 0`, and a 100-byte exchange — more
  than three times the scratch buffer, so a version that only moves `MEMOPS_CHUNK` bytes
  fails. The buffers are exactly their stated sizes with fences around them.
- **`mem_reverse`**: seven cases, including an odd and an even count, `n == 1`, `n == 0`,
  three four-byte elements, five seven-byte elements, and two elements of 33 bytes (one more
  than the scratch buffer).
- **`mem_rotate_left`**: twelve cases, covering `k` of 0, 1, 2, `n - 1`, `n` and `k > n`, an
  even and an odd count, `n == 1`, `n == 0`, and elements of 5 and 40 bytes.
- **Oracle**: 600 generated cases with `n` from 1 to 9, element sizes from 1 to 40 and `k`
  from 0 to 19, each compared against a reference rotation that copies element by element into
  a separate array. A mismatch prints `n`, `size`, `k`, your array and the expected one.
- Every array is a buffer of exactly `n * size` bytes, and after each call the fences are
  checked, so a write one element past the end is caught in the plain build and reported by
  ASan in the sanitizer build.

## Verify

```bash
make check
```

Success ends with `s02-e07-swap-reverse-rotate: all checks passed` and `== check: OK`.

A failing fixed case prints both arrays, one letter per element repeated `size` times:

```
FAIL main.c:49: rotating by 2
       got:  cdeba
       want: cdeab
       first difference at index 3 of 5
```

A failing generated case names the parameters, which is usually enough to see the pattern:
`size` larger than 32 means the chunking is wrong, `k` larger than `n` means the modulo is
missing, and `n == 1` with a large `k` means a special case was forgotten.

In the ASan stage, a `heap-buffer-overflow WRITE` with "0 bytes after" the array means an
index ran to `n` instead of `n - 1`, most often in the reversal's loop.

<details><summary>Hint</summary>

Write `mem_swap` so that it works for any `size`, then `mem_reverse` with it, then read the
rotation's definition again with `mem_reverse` in mind.

</details>
