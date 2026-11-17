# s02-e07-swap-reverse-rotate: notes

## Why it is correct

**`mem_swap`** exchanges in pieces no larger than the scratch buffer:

```c
while (size > 0) {
    size_t chunk = size < sizeof tmp ? size : sizeof tmp;
    memcpy(tmp, pa, chunk);
    memcpy(pa, pb, chunk);
    memcpy(pb, tmp, chunk);
    pa += chunk;  pb += chunk;  size -= chunk;
}
```

Each iteration is the classic three-copy exchange over a bounded piece, so an element of any
size works with 32 bytes of stack. `memcpy` is correct here rather than `memmove` because the
two elements never overlap — the contract says so — and `tmp` is a separate object. The
`pa == pb` test at the top turns the `a == b` case into a no-op rather than a copy of every
byte onto itself.

**`mem_reverse`** swaps from both ends inwards:

```c
for (size_t i = 0; i + 1 < n; i++, n--)
    mem_swap(p + i * size, p + (n - 1) * size, size);
```

`i` rises and `n` falls, so the loop stops when they meet, and `i + 1 < n` is the condition
that is correct for every count: `n == 0` and `n == 1` never enter the body, and an odd count
leaves its middle element alone. Writing `i < n / 2` with the original `n` is equally correct
and needs a second variable for the far index; this form carries the far index in `n` itself.

**`mem_rotate_left`** is the three-reversal trick:

```c
if (n == 0) return;
k %= n;
if (k == 0) return;
mem_reverse(base, k, size);                      /* A -> A' */
mem_reverse(base + k * size, n - k, size);       /* B -> B' */
mem_reverse(base, n, size);                      /* A'B' -> B A */
```

Why it works, in one line: reversing a concatenation reverses the parts and swaps their order,
so `reverse(A' B')` is `reverse(B') reverse(A')` = `B A`, which is the rotation. Each element
is moved exactly twice, so the whole thing is `O(n * size)` with no extra storage.

The two early returns are the arithmetic. `n == 0` has to come first, because `k %= 0` is
undefined (C17 6.5.5p5: division or remainder by zero). `k %= n` then makes every `k` legal:
`k == n` becomes 0, `k == 7` with `n == 5` becomes 2. And `k == 0` returns rather than
reversing three times to no effect — correct either way, but the empty-part reversals would
each be a no-op and the whole-array reversal would not.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `size == 0` in `mem_swap` | `while (size > 0)` never runs |
| `a == b` in `mem_swap` | the early `return` |
| an element larger than the scratch buffer | the chunk loop |
| `n == 0` or `n == 1` in `mem_reverse` | `i + 1 < n` is false immediately |
| an odd count in `mem_reverse` | the middle element is where `i` and `n` meet |
| `n == 0` in `mem_rotate_left` | the first early return, before `k %= n` |
| `k == 0`, `k == n`, `k > n` | `k %= n` and the second early return |
| `n == 1` with any `k` | `k %= 1` is 0, so the second return |
| writing past the array | every index is `< n`, and the far index is `n - 1` |

## Alternatives

Three other ways to rotate, and why the three-reversal version is the one to know:

| Method | Extra space | Element moves | Notes |
|---|---|---|---|
| copy to a second array, copy back | `n * size` | `2n` | simplest; needs the memory |
| rotate by one, `k` times | 0 | `n * k` | fine for `k == 1`, quadratic otherwise |
| cycle-following (juggling) | `size` | `n` | optimal moves, needs `gcd(n, k)` and is fiddly |
| three reversals | 0 | `2n` | no allocation, no number theory, four lines |

For small `k` and a buffer you can afford, the first is clearest; the course's own
`gap_open`/`gap_close` drill (r01) is that shape. The cycle-following method really does move
each element once and is what you want in a tight loop over large arrays — at the cost of a
`gcd` and an inner loop whose bounds are easy to get wrong.

`mem_swap` with `memcpy` through a scratch buffer is what `qsort` implementations do for
element sizes they cannot handle specially. glibc has separate swap routines for 4-byte,
8-byte and "anything else" elements, the last being exactly this loop.

## The usual mistake

Forgetting `k %= n`, so `mem_rotate_left(base, 5, size, 7)` reverses a "first part" of seven
elements in a five-element array. That is a read and a write four elements past the end: a
fence failure in the plain build and a `heap-buffer-overflow` under ASan. The generated cases
produce `k > n` constantly, so this shows up on the first run.

Second: computing `k %= n` before testing `n == 0`. The program then divides by zero, which
UBSan reports and which on some targets is a hardware trap.

Third: a `mem_swap` that only exchanges `min(size, MEMOPS_CHUNK)` bytes. Every test with
small elements passes; the 100-byte case leaves 68 bytes of each element where they were, and
the printed arrays make it obvious once you look at the right one.

Fourth: `mem_reverse` with `i < n / 2` but using the *shrinking* `n`, which halves the work
every iteration and reverses only part of the array. Pick one: shrink `n` and compare
`i + 1 < n`, or keep `n` and compare `i < n / 2`.

## How libc does it

There is no rotation or reversal in the C library — they are algorithms, not primitives, and C
leaves them to you. What the library does provide is the *shape*: `qsort` and `bsearch` take
`(base, nmemb, size)` and a comparison function, which is the same generic-array interface
this exercise uses, and for the same reason: `void *` plus a size is as close to a generic
type as C gets.

Inside `qsort`, glibc swaps elements with code very like `mem_swap`: a specialised path when
the element is the size of a `uint32_t` or a `uint64_t` and the array is suitably aligned, and
a byte-wise loop otherwise. It also chooses between an in-place quicksort and a merge sort
that allocates, depending on the array's size — so the "do I have scratch space?" question
this exercise makes you answer is one the library answers at run time too.

C++'s `std::rotate` is the three-reversal trick (or the cycle-following version, depending on
the iterator category), which is a reasonable argument that it is the canonical in-place
answer.
