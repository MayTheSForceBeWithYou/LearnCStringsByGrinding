# s02-r02-memset-memcmp-reps: notes

## Why it is correct

Three of the four are one line each.

```c
void zero_array(void *base, size_t n, size_t size) { memset(base, 0, n * size); }
int ranges_equal(const void *a, const void *b, size_t n) { return memcmp(a, b, n) == 0; }
```

`n * size` is the total, and `memset` of 0 bytes is legal, so `n == 0` and `size == 0` need no
test. `memcmp(a, b, 0)` returns 0, so two empty ranges come out equal with no special case —
the library's edge cases are already the ones the contract wants.

`common_prefix` is the loop, bounded first:

```c
while (i < n && pa[i] == pb[i])
    i++;
return i;
```

`i < n` before the comparison, so `pa[n]` is never read; `&&` guarantees it (C17 6.5.13p4).
The return value is the index where the loop stopped, which is the count of equal leading
bytes — `0` when the first bytes differ, `n` when all of them match.

`all_equal_to` is the one with a trick worth knowing:

```c
if (n == 0) return 1;
return q[0] == b && memcmp(q, q + 1, n - 1) == 0;
```

If the first byte is `b` and the range equals *itself shifted by one*, then every byte equals
its neighbour, so every byte equals the first, so every byte is `b`. One `memcmp` of `n - 1`
bytes replaces a loop, and it is vectorised. The two ranges `[q, q+n-1)` and `[q+1, q+n)`
overlap, which `memcmp` does not mind: unlike `memcpy`, it only reads.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` in `zero_array` | `n * size` is 0, and `memset` of 0 bytes writes nothing |
| `size == 0` | the same product |
| struct padding | `n * size` covers whole elements, not just members |
| `n == 0` in `ranges_equal` | `memcmp(a, b, 0)` is 0 |
| `n == 0` in `common_prefix` | `i < n` is false at once; 0 |
| `n == 0` in `all_equal_to` | the early `return 1` |
| `n == 1` in `all_equal_to` | `memcmp` of 0 bytes is 0, so the first byte decides |
| a value above 255 | the conversion `(unsigned char)c`, done once |
| bytes above 0x7F | `memcmp` compares as `unsigned char`; nothing here tests a sign |
| reading `pa[n]` | never: the bound is tested first |

## Alternatives

| This function | Other ways |
|---|---|
| `zero_array` | `= {0}` per element, or `calloc` if the memory is being allocated anyway |
| `ranges_equal` | `common_prefix(a, b, n) == n`, which is the same question asked slowly |
| `common_prefix` | word-at-a-time: compare `size_t`s, and on a difference find the byte with a bit scan |
| `all_equal_to` | a plain loop, or `memchr` for "is there a byte that is not `b`" — which `memchr` cannot express, since it searches *for* a byte |

The shifted-`memcmp` trick in `all_equal_to` is worth keeping for the same reason the
`strnlen`/`memchr` identity is: it turns a byte loop into one library call, and the library
call is vectorised. It also has a cost worth knowing — it reads every byte, where a loop could
stop at the first mismatch. For "is this page all zeros" over a megabyte, the loop with an
early exit wins on mismatched input and loses on matched input; the `memcmp` version is
predictable, which is often the better property.

`zero_array`'s interface — `(base, n, size)` rather than `(base, total)` — is the one the
library uses for `qsort`, `bsearch` and `calloc`, and the reason is the same: the caller writes
`sizeof *p` and the two numbers cannot drift apart when the type changes.

## The usual mistake

`memset(base, 0, n)`, forgetting `size`. It clears the first `n` *bytes* of an array of `n`
*elements*, which for a 24-byte struct means the first three elements of twenty-four are
partly cleared. Nothing is out of bounds, so no tool says anything, and the records that were
not cleared carry their old bytes forward — e04 of this unit, by a different route.

Second: `common_prefix` with the conditions in the wrong order
(`pa[i] == pb[i] && i < n`), which reads `pa[n]`. The harness's exactly-sized buffers turn
that into an ASan report.

Third: `all_equal_to` written as `memcmp(p, p, n) == 0`, which is trivially true, or as a
comparison against a filled scratch buffer — correct, and it needs a buffer of `n` bytes,
which is the thing to avoid.

Fourth: `ranges_equal` returning `memcmp(a, b, n)` rather than `memcmp(a, b, n) == 0`. The
function is documented to return 1 or 0, and `memcmp`'s value is only specified in its sign;
returning it directly gives a function whose "true" is sometimes 0 and sometimes -1.

## How libc does it

`memset` broadcasts the byte into a vector register and stores 32 or 64 bytes at a time;
`memcmp` compares a register's worth at a time and only works out which byte differed when it
has to. Both are selected at load time from the processor's feature bits.

One property of `memcmp` that matters outside this drill: it is allowed to read all `n` bytes
even after it could have stopped, and a vectorised version does. So `memcmp` is **not**
constant-time, and `ranges_equal` built on it must not be used to compare a password hash or
a MAC. The constant-time version ORs the XOR of every byte pair and compares the total with
zero once — more work, by design.

`calloc` is the other half of `zero_array`'s job and is worth preferring when the memory is
being allocated anyway: on a fresh page the kernel has already zeroed it, so `calloc` can skip
the write entirely, which `malloc` plus `memset` cannot.
