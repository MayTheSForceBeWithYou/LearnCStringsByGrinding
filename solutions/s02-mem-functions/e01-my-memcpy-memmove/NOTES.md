# s02-e01-my-memcpy-memmove: notes

## Why it is correct

`my_memcpy` is a forward loop over `unsigned char`:

```c
unsigned char *d = dst;
const unsigned char *s = src;
for (size_t i = 0; i < n; i++)
    d[i] = s[i];
return dst;
```

The caller has promised the regions do not overlap, so the order does not matter and forwards
is as good as any. `unsigned char` is the type the standard lets you inspect the bytes of any
object through (C17 6.5p7), and it keeps the signedness of plain `char` out of a function that
never looks at a value anyway.

`my_memmove` picks the direction from the two addresses:

```c
if (d < s) {                       /* forwards: read a byte before it can be overwritten */
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
} else if (d > s) {                /* backwards, for the same reason mirrored */
    for (size_t i = n; i-- > 0;)
        d[i] = s[i];
}
```

Why that is right, in one sentence each. If `d < s`, then for every `i` the address `d + i` is
below `s + i`, so by the time the loop writes `d[i]` it has already read every source byte at
or below that address — nothing still needed has been clobbered. If `d > s`, the same argument
runs from the top: `d + i` is above `s + i`, so writing from the highest index down never
touches a source byte the loop has not read. If `d == s` there is nothing to do, and the
`else if` leaves the bytes alone rather than copying each one onto itself.

`for (size_t i = n; i-- > 0;)` is the backward loop that is correct for `n == 0`: the test
decrements after comparing, so with `n == 0` the comparison `0 > 0` fails and the body never
runs. Writing `for (size_t i = n - 1; i >= 0; i--)` instead would loop forever on an unsigned
index, and `n - 1` with `n == 0` is `SIZE_MAX` — S01 e03's defect in a new place.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` | both loops' conditions fail at once; nothing is read or written |
| `dst == src` | the `else if`: neither branch runs |
| a one-byte overlap, dst above src | the backward branch |
| a one-byte overlap, dst below src | the forward branch |
| completely disjoint regions | either branch is correct; the comparison picks one anyway |
| bytes above 0x7F, NUL bytes | nothing inspects a byte's value |
| the return value | `return dst;` in both, the original pointer, not the advanced one |

## Alternatives

A pointer-walking form, which is what you will see in libc sources:

```c
while (n-- > 0)
    *d++ = *s++;
```

Identical in effect and slightly shorter; the index form is used here because the backward
branch is clearer with an index than with a pointer that has to start at `d + n` and
pre-decrement.

For the direction test, `if (d < s)` compares two pointers that may point into *different*
objects, and C17 6.5.8p5 says relational comparison of such pointers is undefined. In
practice every implementation compares addresses, and every real `memmove` does exactly this;
a strictly conforming version would compare `(uintptr_t)d < (uintptr_t)s`, which is
implementation-defined but not undefined. This is one of the places where the library is
allowed to know more about the machine than a portable program may assume — the lesson's
*What the tools say* comes back to it.

The library call that replaces the whole exercise is, of course, `memmove`. Use it: it is
vectorised, it has special cases for small sizes, and on x86-64 it may move 32 or 64 bytes per
instruction. Writing it once, by hand, is for knowing what the direction test is *for*.

## The usual mistake

Copying forwards always. The function then passes every disjoint test, every `dst > src`
test, and fails whenever the destination is above the source and the regions overlap — which
is exactly the shape of "insert a gap in a buffer", the most common reason to call `memmove`
at all. The result is the first few bytes repeated across the destination, which looks like
corrupted data rather than a wrong direction. The sweep in this harness exists to catch it on
the first run.

Second: `for (size_t i = n - 1; i >= 0; i--)` for the backward loop. Infinite, and wrong for
`n == 0` twice over.

Third: implementing `my_memmove` with a temporary buffer (`char tmp[n]` or a `malloc`). It is
correct, it is what the standard's "as if" describes, and it is not what the function is for:
a variable-length array of the caller's size can be megabytes on a stack that has kilobytes,
and an allocation can fail in a function whose contract has no failure. The direction test
costs one comparison.

Fourth: forgetting that `my_memcpy` must still return `dst` after advancing its working
pointers. Keep the parameter and return it, or save it first.

## How libc does it

Neither function is a byte loop in any libc you will link against. glibc selects an
implementation at load time from the processor's feature bits (an *ifunc*): on x86-64 that is
typically an AVX2 or AVX-512 routine copying 32 or 64 bytes per iteration, with separate paths
for sizes under 16 bytes, for sizes that fit in a few vector registers, and for very large
copies where it uses non-temporal stores to avoid filling the cache with data nobody will read
again.

`memmove` shares most of that code and adds the direction test. For large overlapping copies
it may copy in blocks from the appropriate end rather than byte by byte, which is why the
"as if a temporary" wording exists in the standard: it describes the *result*, not the method.

One historical detail worth keeping, because it is the best argument against relying on
undefined behaviour. In glibc 2.13 the x86-64 `memcpy` changed the order in which it copied
bytes, for speed. Programs that had been passing overlapping buffers to `memcpy` — and getting
away with it, because the old implementation happened to copy forwards — broke. glibc 2.14
added a versioned symbol so that binaries linked against the older library kept the old
behaviour. The Linux `memcpy(3)` page tells the story in its CAVEATS section, and e08 asks you
to read it.
