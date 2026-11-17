# s01-e06-narrow-length: notes

## Why it is correct

The fixed test is on the quantity the conversion cares about:

```c
if (len > NAME_MAX)
    return -1;
```

`NAME_MAX` is 255, the largest value an `unsigned char` can hold, so after the test the
conversion `(unsigned char)len` cannot lose information. The test comes before any write, so a
rejected name leaves the record byte-for-byte as it was — which is what the header promises
and what `main.c` checks with a `memcmp` of the whole struct.

Note what the fix does *not* do. It does not widen `len`: the one-byte length is the wire
format and is not the program's choice. It does not clamp the length to 255 and store a
truncated name, which would be a silent data loss of a different shape. And it does not test
`len + 1 > NAME_MAX`, which would reject a perfectly storable 255-character name: the length
field holds a *length*, and the terminator in the buffer is not part of it.

The `NAME_CAP` test is gone because it is now implied: a name of at most 255 characters plus a
terminator is 256 bytes, and the buffer is 512. If the two constants could ever drift apart,
the honest form is to keep both tests, or to assert the relationship once:

```c
/* NAME_MAX + 1 <= NAME_CAP, or the buffer cannot hold what the format allows */
```

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty name | `0 > 255` is false; `memcpy` of one byte; `len` is 0 |
| a 255-character name | `255 > 255` is false: accepted, and the conversion is exact |
| a 256-character name | `256 > 255` is true: rejected before anything is written |
| a 300-character name | the same test |
| a 511-character name | the same test, and the buffer is never consulted |
| a rejected call | the early `return -1`, which is before the `memcpy` |
| the stored terminator | `memcpy(n->buf, s, len + 1)` copies it, so `buf` is also a string |

## Alternatives

The same check, written three other ways:

```c
if (len > (size_t)UCHAR_MAX)                 /* <limits.h>: the type's own maximum   */
if (len != (size_t)(unsigned char)len)       /* "does it survive the round trip?"    */
if (len > NAME_MAX) ...                      /* the format's maximum (what we wrote) */
```

The third is the best of the three *here*, because it names the constraint the format imposes
rather than the type that happens to implement it. The second is the general form worth
knowing: "convert, convert back, compare" is the portable test for any narrowing conversion,
and it is what a checked-conversion helper does in code bases that have one.

A design that removes the problem rather than checking it: store the length as a `size_t` in
memory and convert only at the point where the record is serialised, where the check lives
once for the whole format. That is usually right for a program that reads and writes a format,
and it is wrong for one that maps the format directly onto a struct — which is what this
record does, and what a great deal of embedded and protocol code does.

## The usual mistake

Testing against the buffer. It is the number in front of you (`NAME_CAP` is in the struct's
declaration), it is a real constraint, and it is not the binding one. Any time a program has
two bounds — what the storage can hold and what the *representation* can describe — the
smaller one is the contract, and it is usually the one not written next to the buffer.

Second: clamping instead of rejecting.

```c
if (len > NAME_MAX)
    len = NAME_MAX;              /* now the record holds a truncated name */
```

The record is self-consistent again, and 45 characters of the user's name are gone with no
return value saying so. For a display label that may be acceptable; for a name, a path or a
key it is the "truncate silently" policy that S03 e06 is about, and the caller never found
out.

Third: fixing it in `name_get` — noticing that `strlen(n->buf)` disagrees with `n->len` and
preferring the `strlen`. That makes `name_get` return 300 characters out of a record whose
format says 44, so the in-memory record and the serialised one now differ. The reader was
right to trust the length field; the writer was wrong to store a length it could not hold.

## How libc does it

The C library has nothing to say about narrowing conversions, and that absence is the lesson:
`strlen` returns the widest unsigned type the implementation has, and what you do with that
value at a format boundary is entirely your problem. `<limits.h>` and `<stdint.h>` are the
tools — `UCHAR_MAX`, `UINT8_MAX`, `SIZE_MAX` — and the C23 additions in this area
(`ckd_add` and friends, for *arithmetic* overflow) still do not cover assignment.

Where the library does model the problem well is in the functions that return a length as an
`int`: `snprintf` returns `int`, so output longer than `INT_MAX` is an error it reports with a
negative value rather than a wrapped one (S11). That is the shape to copy — a representable
range, a documented maximum, and an error return for anything outside it — and this exercise's
`name_set` now has all three.
