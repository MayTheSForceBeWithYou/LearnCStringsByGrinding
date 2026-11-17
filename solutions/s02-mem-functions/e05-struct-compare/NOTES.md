# s02-e05-struct-compare: notes

## Why it is correct

The fix compares what the header says a point is:

```c
int point_eq(const Point *a, const Point *b)
{
    return a->kind == b->kind && a->value == b->value;
}
```

Two members, two comparisons, no bytes that belong to nobody. It is correct whatever the
padding holds, whatever the compiler's alignment rules are, and on a platform where the struct
is laid out differently.

`point_copy` keeps its `memcpy` and that is right. A copy that carries the padding along is
harmless — the padding of the copy is as unspecified as the padding of the original — and it
is exactly what plain struct assignment (`*dst = *src;`) does. The asymmetry is the thing to
remember: **copying all the bytes is fine, comparing all the bytes is not.**

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| same members, different padding | the member comparison; the padding is never read |
| different `kind` | `a->kind == b->kind` |
| different `value` | the second clause |
| a point compared with itself | both clauses true |
| a copy compared with its original | `point_copy` copies the members; the comparison reads them |
| a point from a differently-built table | nothing in the comparison depends on how it was built |

## Alternatives

Three ways to make byte-wise comparison safe, in increasing order of how much they ask of the
rest of the program:

1. **Clear before filling**: `memset(&p, 0, sizeof p)` and then assign the members, everywhere
   a `Point` is created. Every point then has zero padding and `memcmp` works. This is what
   code that must hash or serialise structs does — and it is a whole-program obligation: one
   place that forgets, one struct read from a socket, one `realloc`ed array, and the
   comparison is silently wrong again.
2. **Make the padding impossible**: order the members so that no padding is needed, or give
   the struct a fixed layout for serialisation and convert at the boundary. Reordering is
   fragile (it depends on the platform's alignment) but it is why `int` members often come
   first in C structs.
3. **Compare members.** What the fix does. It is immune to layout, it says what equality
   means, and the compiler usually generates the same two instructions.

For a struct of many members, writing the comparison out gets tedious, and that is a real
argument for a generated or macro-written comparison — not for `memcmp`.

The related rule for hashing: hashing a struct with `memcmp`-style byte access has exactly the
same problem, and it is worse, because a hash table with padding in its keys fails
*intermittently*. The same three options apply.

## The usual mistake

`memcmp` on a struct, which is this exercise. It is attractive because it is one line and
generic, it passes every test where both objects were built the same way, and it is wrong for
a reason that is invisible in the source.

Second: "fixing" it by zeroing the padding in `point_make`. The harness would then pass, and
every `Point` built anywhere else — by a caller that declares one on the stack and assigns its
members, by a `memcpy` from a file, by `calloc` plus two assignments — is still liable to
differ. The constructor cannot police the type, because C does not force values of a type
through their constructor.

Third: concluding that `memcmp` is dangerous and should be avoided. It is exactly right for
the thing it says it does: comparing `n` bytes. Arrays of `char`, buffers from the network,
fixed-width fields, a block of pixels — all of those are byte-wise values and `memcmp` is the
tool. The question is whether the object's bytes *are* its value.

## How libc does it

Nothing in libc knows what a struct is; `memcmp` receives a `const void *` and a count. That
is the whole point of the `mem` family: they are byte primitives, and the meaning of the bytes
is the caller's business.

The C standard's own interfaces are careful about this in a way that is worth copying. There
is no standard "compare two structs" and no standard hash; `qsort` and `bsearch` take a
comparison *function*, so the caller says what equality and ordering mean for its own type
(S06 e04). Every library that does compare aggregates for you — in other languages — either
generates a member-wise comparison or requires the type to have no padding.

One C23 note: the new `memset_explicit` is about a different problem (bytes a compiler may
optimise away), and nothing in C23 changes padding's unspecified values. What C23 does add is
a clearer statement that padding bytes take unspecified values when a member is assigned,
which is the sentence this exercise rests on.
