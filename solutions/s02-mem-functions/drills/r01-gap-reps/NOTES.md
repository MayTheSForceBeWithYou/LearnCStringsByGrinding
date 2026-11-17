# s02-r01-gap-reps: notes

## Why it is correct

Each function is a test, then one move, then the new length.

```c
size_t gap_open(char *buf, size_t cap, size_t len, size_t at, size_t n)
{
    if (at > len || n > cap - len)
        return len;
    memmove(buf + at + n, buf + at, len - at);
    return len + n;
}
```

`len - at` is the number of bytes that have to move: everything from `at` to the end of the
used region. The destination is `at + n`, which is above the source, so the regions overlap
and `memmove` is required — a `memcpy` here is the defect of e03.

The test is written as `n > cap - len` rather than `len + n > cap` on purpose. `len <= cap`
always holds, so `cap - len` cannot wrap; `len + n` can, and if it does the test passes and
the move runs off the end of the buffer. When a sum of two `size_t` values might overflow,
move one term to the other side.

`gap_close` is the mirror image:

```c
if (at > len || n > len - at)
    return len;
memmove(buf + at, buf + at + n, len - at - n);
return len - n;
```

`len - at - n` is what is left after the deleted range: the tail that moves down. Both
subtractions are safe because the test has already established `at <= len` and `n <= len - at`.
The destination is below the source here, so a forward loop would also work — and `memmove` is
still the right call, because the next person to edit this line may be writing the other
direction.

`gap_delete` converts a range to an offset and a count and delegates; `gap_insert` opens the
gap and fills it with `memcpy`, which is correct because `src` does not overlap `buf`. Its one
subtlety is distinguishing "the open failed" from "the open moved nothing":

```c
size_t after = gap_open(buf, cap, len, at, n);
if (after == len && n > 0)
    return len;
memcpy(buf + at, src, n);
return after;
```

`gap_open` returns `len` both when it refuses and when `n` is 0, so the `n > 0` guard is what
keeps a legitimate zero-byte insert from being treated as a failure.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` | the move's count becomes a no-op move; the length is unchanged |
| `at == len` (at the end) | `len - at` is 0, so nothing moves, and the length still grows |
| a request that exactly fills the buffer | `n > cap - len` is false when `n == cap - len` |
| a request one byte too large | the same test, now true: nothing is written |
| `at > len` | the first clause of each test |
| `from > to` in `gap_delete` | its own test, before the subtraction `to - from` |
| `to > len` | the second clause, before anything moves |
| `len == 0` | every count is 0; the tests still hold |
| overlapping regions | `memmove`, in all three moves inside `buf` |
| `src` not overlapping `buf` | `memcpy` in `gap_insert`, which is allowed to assume it |

## Alternatives

The same four operations with a second buffer:

```c
memcpy(out, buf, at);
memcpy(out + at, src, n);
memcpy(out + at + n, buf + at, len - at);
```

Three disjoint copies, no overlap anywhere, and obviously correct at a glance — at the cost of
`cap` more bytes and a second pass. That is the right shape when you have the memory and the
code is cold; the in-place version is what a text buffer or a dynamic array does, because it
is called on every keystroke.

A real gap buffer (D20 of the data structures course, and S19 here) avoids most of these moves
altogether by keeping the gap *at the cursor* and only moving it when the cursor moves, which
turns a sequence of insertions at one place from O(n) each into O(1) each. The primitives are
still these four.

For the arithmetic, the alternative to "move a term across" is to check for overflow
explicitly (`if (n > SIZE_MAX - len)`), which is what you need when neither operand is bounded.
Here `len <= cap` is an invariant of the interface, so the simpler form is available and worth
preferring: one subtraction that cannot wrap beats one addition that can.

## The usual mistake

`memcpy` instead of `memmove` for the moves inside `buf`. It is undefined behaviour in every
one of them, it produces the right answer for `gap_close` on every implementation anyone has
shipped, and it produces visible garbage for `gap_open` — so a program with this defect
usually has it in only the half you do not notice. ASan reports it as
`memcpy-param-overlap`, which is why the drill's checks run under the sanitizer too.

Second: `len + n > cap` as the fits-test. For ordinary sizes it is right; for a `len` or `n`
near `SIZE_MAX` it wraps and admits a move that destroys the heap. The harness does not pass
such values — real callers do, when a length arrives from a file or a network.

Third: computing `len - at - n` before the test that guarantees `at + n <= len`. One
unsigned subtraction too early turns a rejected request into a move of about 18 quintillion
bytes.

Fourth: clearing the gap in `gap_open`. It is a second pass over bytes the caller is about to
overwrite, and it hides the bug where the caller forgets to fill it — a gap full of zeros
looks much more plausible than a gap full of the old text.

## How libc does it

`memmove` is the whole implementation here, and the thing worth knowing about it is that the
direction test is its only difference from `memcpy`: both are selected at load time from the
processor's feature bits, both copy 32 or 64 bytes per instruction on x86-64, and for small
sizes both may load everything into registers before storing anything — which makes overlap
irrelevant and is why so much overlapping `memcpy` code gets away with it.

C has no "insert into a buffer" primitive, and neither do POSIX or the BSDs. What exists is
`memmove` and arithmetic, which is why this drill is four tiny functions rather than a library
lookup. The nearest thing to a standard interface in the area is C++'s `std::vector::insert`
and `std::rotate`; both are implemented over exactly these moves, and `std::rotate` is e07 of
this unit.
