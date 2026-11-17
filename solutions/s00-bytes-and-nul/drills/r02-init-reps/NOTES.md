# s00-r02-init-reps: notes

## Why it is correct

All three writers share one shape: return early if `cap == 0`, loop while there is both
something to write and room for it plus a terminator, then terminate at the index the loop
left behind.

```c
for (i = 0; i < n && i + 1 < cap; i++)
    buf[i] = c;
buf[i] = '\0';
```

`i + 1 < cap` is the whole drill. It says "there is room for this character and still a byte
for the terminator", so when the loop ends, `buf[i]` is a byte inside the buffer and writing
the terminator there cannot overflow. Written as `i < cap - 1` it means the same for every
`cap` greater than 0 — and `cap == 0` is already gone — but the `i + 1` form is the one that
stays correct if the early return is ever removed, because `cap - 1` with an unsigned 0 is
`SIZE_MAX`.

`init_literal` has the same loop with `lit[i] != '\0'` as its other condition, so it stops at
whichever comes first, the end of the source or the end of the room. It therefore reads the
source exactly once and never reads past the byte that did not fit.

`init_terminated` is bounded by `cap` and tests before it reads, so `cap == 0` reads nothing.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the early return in all four (and `i < cap` being false in the fourth) |
| `cap == 1` | `i + 1 < cap` is false at `i == 0`, so only the terminator is written |
| `n == 0` | `i < n` is false at `i == 0` |
| `n == SIZE_MAX` | `n` is only compared, never incremented |
| an empty source | `lit[0] == '\0'` ends the loop at once; `buf` becomes `""` |
| a source that fits exactly | the loop ends on the source, not on the room |
| a source one byte too long | the loop ends on the room, and the return value is `cap - 1` |
| a byte above 0x7F | stored as a `char` without inspection |
| bytes past the terminator | never written: the loop writes `[0, i)` and the terminator at `i` |

## Alternatives

With the library, once you have the units:

| This function | The library |
|---|---|
| `init_empty` | `buf[0] = '\0';` after checking `cap > 0` — there is no call shorter than this |
| `init_repeat` | `memset(buf, c, n); buf[n] = '\0';` with `n < cap` checked first (S02) |
| `init_literal` | `snprintf(buf, cap, "%s", lit)`, which also reports how much it needed (S03, S11) |
| `init_terminated` | `memchr(buf, '\0', cap) != NULL` (S02), or `strnlen(buf, cap) < cap` (S01) |

The `memset` version is much faster per byte than the loop, and the `snprintf` version is
much slower than either, because it parses a format string on every call. All three are
correct; which one belongs in a given line depends on whether the cost is in a hot loop or in
error handling that runs once.

Note that `init_repeat`'s return value makes truncation visible (`< n` means cut), while the
`memset` version makes the caller decide before writing. That is the S03 e06 question —
truncate, or refuse — and this drill deliberately only truncates.

## The usual mistake

Bounding the loop by `cap` instead of `cap - 1`, so the terminator lands on `buf[cap]`. The
harness catches it three ways at once: a wrong return value, a last byte that is not `\0`, and
a fence or ASan report for the byte outside. The same off-by-one is the whole content of
S18 e01 and S18 e03, and in production it is the overwritten saved register next to your
buffer.

Second: writing the terminator unconditionally before the `cap == 0` test, which overflows a
zero-size buffer by exactly one byte — the case nobody tests by hand, and the reason the
harness passes `cap == 0` to all four.

Third: `init_literal` measuring the source first and refusing when it does not fit. That is a
*different function* (a strict copy, S03 e06's `copy_strict`), and it is a perfectly good one;
it just is not this header's contract. Read the contract, then write the loop.

## How libc does it

`memset` is the production form of `init_repeat`'s loop and is one of the most heavily
optimised functions in any libc: wide vector stores, a separate path for small sizes, and on
x86-64 sometimes `rep stosb`, which the processor implements in microcode faster than any
loop you can write. `memcpy`, the production form of `init_literal`'s loop, is the same story
with two pointers.

Nothing in that machinery knows about terminators. `memset` and `memcpy` take a byte count and
write exactly that many bytes, which is why every one of these four functions ends with a line
*you* wrote putting the terminator in. The library's division of labour — it moves bytes, you
maintain the convention — is the reason S02 comes before S03, and the reason a bounded copy in
C is always two decisions: how many bytes, and who writes the zero.
