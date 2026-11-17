# s02-r03-mem-from-scratch: notes

## Why it is correct

Five functions, three shapes.

**The copies.** `s_memcpy` is a forward loop over `unsigned char`; the caller promised the
regions are disjoint, so the order does not matter. `s_memmove` compares the two pointers and
copies forwards when the destination is lower and backwards when it is higher:

```c
if (d < s)        for (size_t i = 0; i < n; i++)   d[i] = s[i];
else if (d > s)   for (size_t i = n; i-- > 0;)     d[i] = s[i];
```

Forwards is safe when `d < s` because every byte is read before the write that would overwrite
it; backwards is the mirror argument. `d == s` falls through both branches and does nothing,
which is both correct and faster than copying each byte onto itself.

`for (size_t i = n; i-- > 0;)` is the backward loop that works for `n == 0`: the comparison
happens before the decrement takes effect, so `0 > 0` ends it immediately. The tempting
`for (size_t i = n - 1; i >= 0; i--)` never ends and starts at `SIZE_MAX` when `n` is 0.

**The fill.** `s_memset` converts once and stores:

```c
unsigned char b = (unsigned char)c;
for (size_t i = 0; i < n; i++) p[i] = b;
```

**The inspections.** `s_memcmp` returns on the first difference, comparing `unsigned char`
values, so `0xFF` is 255; `s_memchr` compares against the converted byte and returns a pointer
on the first match and NULL after the loop. Both test `i < n` before touching a byte, so
neither reads `p[n]`.

Every one of the five uses `unsigned char` because that is the type the standard allows for
inspecting the bytes of any object (C17 6.5p7), and because it removes the signedness of plain
`char` from functions that must not depend on it.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` in all five | every loop's condition is false at once |
| `dst == src` in `s_memmove` | the `else if`, which does nothing |
| a one-byte overlap either way | the direction test |
| `c` outside 0..255 | the single conversion to `unsigned char` |
| bytes above 0x7F in `s_memcmp` | the `unsigned char` comparison |
| a NUL byte | nothing in these five stops at a zero byte; only `n` ends them |
| the return values | `dst`, `dst`, `s`, a sign, a pointer or NULL — four different shapes |

## Alternatives

Everything in e01's and e02's notes applies. What is worth adding for a drill whose point is
speed of writing: the five functions have a *shared skeleton*, and knowing it is what gets the
time under fifteen minutes.

```
unsigned char *p = <pointer>;          /* one or two of these */
unsigned char b = (unsigned char)c;    /* for the two that take a byte */
for (size_t i = 0; i < n; i++)         /* or the backward form */
    ...one line...
return <dst, s, 0, or NULL>;
```

Write the skeleton first, five times, then fill in the one line and the return. The only
function that needs more than that is `s_memmove`, which needs the comparison and the second
loop.

The library identity worth carrying out of the unit: `strnlen(s, n)` is
`(memchr(s, '\0', n) ?: s + n) - s`, so once you have `s_memchr` you have the bounded length
too, and S01's two functions are both one line over these five.

## The usual mistake

Under time pressure, three:

`s_memmove` forwards only. It passes every disjoint case and fails exactly when the
destination is above the source — the "insert a gap" direction, which is most of what
`memmove` is called for. The sweep catches it immediately.

`s_memcmp` with `char` pointers, so `0xFF` is -1. Every ASCII test passes; the first byte
above 0x7F sorts in the wrong place. If you find yourself writing `return pa[i] - pb[i];`,
check what `pa` points to.

Forgetting the conversion in `s_memset` or `s_memchr`, which only shows up for arguments
outside 0..255 — `memset(p, -1, n)` and `memchr(s, 0x141, n)` being the two the harness uses.

And one that costs a rep rather than a check: returning the *advanced* pointer instead of the
original. Save `dst` (or keep the parameter untouched and walk a copy) before the loop.

## How libc does it

All five are vectorised and selected at load time from the processor's feature bits, so the
`memcpy` your program calls is chosen by the CPU it starts on. Typical shapes on x86-64: 32-
or 64-byte vector loads and stores with special paths for small sizes; `memset` broadcasting
the byte into a register; `memchr` comparing a whole register and finding the first set bit of
the mask; `memcmp` comparing registers and only identifying the byte when it must.

Two consequences this drill's contracts hint at. `memcmp` may read all `n` bytes even after a
difference, so it is not constant-time and is wrong for secrets. And the vector
implementations are allowed to read a few bytes past what you asked for, inside an aligned
block they have already touched — something your version must never do, which is why every
buffer in this harness is exactly the size the call may touch.
