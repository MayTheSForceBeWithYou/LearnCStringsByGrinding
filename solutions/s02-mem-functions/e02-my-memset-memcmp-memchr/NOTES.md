# s02-e02-my-memset-memcmp-memchr: notes

## Why it is correct

All three begin by converting their arguments to the one type that may look at an object's
bytes:

```c
unsigned char *p = s;
unsigned char b = (unsigned char)c;
```

That single conversion settles three questions at once. The fill byte is the low eight bits of
`c`, which is what `memset(p, 0x141, n)` storing `'A'` means. The comparison is unsigned, so
`0xFF > 0x01` on every platform. And the search compares like with like, so `memchr(s, 0x141
+ 0x14, n)` looks for the byte `0x55`, not for some 32-bit value.

`my_memcmp` returns on the first difference:

```c
if (pa[i] != pb[i])
    return pa[i] < pb[i] ? -1 : 1;
```

Returning -1 and 1 rather than `pa[i] - pb[i]` is a choice, not a requirement: only the sign
is specified. The subtraction form is also correct here — both operands are promoted to `int`,
and the difference of two values in 0..255 always fits — but it gets written as
`*pa - *pb` with `char` pointers by people who have not thought about it, and then it is
wrong. The explicit comparison cannot be.

`my_memchr` returns `(void *)(p + i)`, casting away the `const` its parameter carried. That is
what the library does too, and it is the C way of saying "the constness of the result is the
caller's business": `memchr` on a `const char *` hands you a `char *` pointing into a buffer
you promised not to write. C23 changes this for `strchr` and friends with a type-generic
trick; C17 does not, and S08 e05 is the defect that follows from it.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` everywhere | every loop's `i < n` is false at once; `my_memcmp` returns 0, `my_memchr` NULL |
| a zero-byte buffer | the same test, so no byte is dereferenced |
| `c` outside 0..255 | the conversion `(unsigned char)c`, done once |
| `c == 0` | nothing special: it is a byte value like any other |
| bytes above 0x7F | `unsigned char` comparison, so 0xFF is 255 and not -1 |
| an embedded NUL | no loop stops at a zero byte; only `n` ends them |
| a difference past `n` | the loop never reaches it |
| the first match of several | `my_memchr` returns on the first, not the last |

## Alternatives

| This function | Written another way |
|---|---|
| `my_memset` | a word-at-a-time fill: broadcast the byte into a `size_t` and store whole words, with the ends done byte by byte |
| `my_memcmp` | compare word-at-a-time, and only on a difference work out which byte it was |
| `my_memchr` | the SWAR trick: XOR a broadcast word with the data and test for a zero byte with `(w - 0x0101..) & ~w & 0x8080..` |

All three are what a real libc does, and all three are forbidden here in spirit as well as by
`FORBID`: the point of this exercise is the contract, not the throughput. They are worth
knowing about because they explain the one thing the library is allowed to do that you are
not — read a few bytes beyond what you asked for, inside an aligned block it has already
touched.

The useful identity to carry away is `strnlen(s, n) == (memchr(s, '\0', n) ?: s + n) - s`,
which the Linux `strnlen(3)` page gives: a bounded length *is* a bounded search for a zero
byte. After this unit, the bounded tools are all one function.

## The usual mistake

Comparing as `char`:

```c
const char *pa = a, *pb = b;
return pa[i] - pb[i];     /* -1 for 0xFF on this platform */
```

Every test with ASCII data passes. The first byte above 0x7F — a UTF-8 sequence, a Latin-1
accent, a byte from a binary file — sorts in the wrong place, and the defect shows up as a
binary search that misses a key that is certainly in the table. The harness has four cases for
it and compares against libc's sign as well, because the point is not "some negative number"
but "the same order libc uses".

Second: `memset` with the value and the count swapped. It compiles, because both parameters
are integers; e04 is a program that did it.

Third: `my_memchr` written as a loop that ends with `return p + i;` outside the `if`, so a
search that finds nothing returns a pointer one past the end instead of NULL. The caller then
reads a byte that is not there, and the report names the caller.

Fourth: converting `c` inside the loop (`if (p[i] == (unsigned char)c)`). Correct, and it
repeats the conversion `n` times; worth noticing because it is the same instinct that makes
`strlen` appear in a loop condition.

## How libc does it

`memset` and `memchr` get the full vector treatment: on x86-64 glibc broadcasts the byte into
a 256- or 512-bit register and stores or compares a register's worth at a time, with separate
paths for small sizes. `memchr` turns a comparison into a mask and finds the first set bit, so
a search over a kilobyte costs about thirty instructions rather than a thousand.

`memcmp` is the interesting one, because its contract constrains the optimisation more than
you would expect. It may stop at the first difference — but it is also allowed to read all `n`
bytes, and a vectorised version does, which is why `memcmp` is **not** a constant-time
comparison and must never be used to check a password or a MAC. (For that you want a loop that
ORs the differences of every byte and compares the total with zero once.) The standard says
nothing about timing, and the library promises nothing.

One more consequence of "only the sign is specified": glibc's x86-64 `memcmp` returns the
difference of the two differing bytes, while some other implementations return -1/0/1, and
versions of glibc have changed which. Code that compares the result with 1 is relying on
something no one ever promised — the same shape of mistake as relying on `memcpy` with
overlapping arguments.
