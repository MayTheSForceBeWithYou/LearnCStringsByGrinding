# s01-e02-fixed-fields: notes

## Why it is correct

All three functions start from the same measurement, and that is the design:

```c
size_t field_len(const char *field, size_t width)
{
    return strnlen(field, width);
}
```

`strnlen` is exactly this function's contract — the length before the first NUL, or `width`
when there is none — so there is nothing to write. Saying so with a named function is still
worth it: the name records what the number means in this program, and the other two functions
then read as prose.

`field_eq` compares lengths first and then only the text bytes:

```c
size_t la = strnlen(a, width), lb = strnlen(b, width);
return la == lb && memcmp(a, b, la) == 0;
```

The length test is not an optimisation; it is part of the answer. Without it, `ab` and `a\0`
in a 2-byte field would compare equal on their first `la == 1` byte. And `memcmp` is given the
measured length, never `width`, so padding is never compared. `memcmp(a, b, 0)` is defined and
returns 0, which makes two empty fields equal and `width == 0` equal, with no special case.

`field_to_cstr` measures once, then copies the part that fits and terminates:

```c
size_t len = strnlen(field, width);
if (cap > 0) {
    size_t keep = len < cap - 1 ? len : cap - 1;
    memcpy(dst, field, keep);
    dst[keep] = '\0';
}
return len;
```

`cap > 0` is tested before anything is written, so a zero-byte destination is untouched.
`keep` is `min(len, cap - 1)`, which reserves the terminator's byte, and `dst[keep]` is
therefore always inside the buffer. The return value is `len`, not `keep`: the length the
function *tried* to produce, which is what makes truncation detectable and what lets a caller
ask for the required size with `cap == 0`.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| a full field (no terminator) | `strnlen`'s bound; `field[width]` is never read |
| `width == 0` | `strnlen(field, 0)` is 0 and reads nothing; `memcmp(a, b, 0)` is 0 |
| junk after the first NUL | the measurement stops there, and `memcmp`/`memcpy` get `len` |
| two fields of different lengths | `la == lb` short-circuits before `memcmp` |
| `cap == 0` | the `if (cap > 0)` guard, before any write |
| `cap == 1` | `keep` is `min(len, 0)` = 0, so only the terminator is written |
| an exact fit | `keep == len` and `dst[len] = '\0'` is the last byte of `dst` |
| truncation | `keep == cap - 1`, and the return value is still `len` |
| bytes above 0x7F | `memcmp` compares as `unsigned char`; nothing here tests a sign |

## Alternatives

| This function | Written with the library |
|---|---|
| `field_len` | `strnlen(field, width)`, which is all it is |
| `field_eq` | no single call: `strncmp(a, b, width)` is **not** the same (see below) |
| `field_to_cstr` | `snprintf(dst, cap, "%.*s", (int)width, field)` (S11), which has the same contract and the same return value |

`strncmp(a, b, width)` is the tempting one-liner for `field_eq`, and it is wrong in one
direction and right in another. It stops at a terminator in either string, so `ada\0\0\0\0\0`
and `ada\0xxxx` *do* compare equal — good — but on two full fields it compares all `width`
bytes and gives an ordering rather than an equality, which is fine, and on `ab` against `a\0`
it also gives the right answer. The real objection is subtler: `strncmp` reads at most `width`
bytes but its contract is about *strings*, so using it on data that may not be terminated
works by accident of the bound rather than by design. S06 e08 builds the comparison family
properly.

The `%.*s` form of `field_to_cstr` is worth remembering for later: `printf("%.*s", (int)width,
field)` prints a field with no copy and no terminator at all, which is often all you wanted.
Note the `int` cast and its range, which S11 covers.

## The usual mistake

Comparing all `width` bytes:

```c
return memcmp(a, b, width) == 0;    /* the padding is now part of the value */
```

This passes every test whose fields were written by the same program, because that program
zeroes the padding. It fails the moment a record comes from somewhere else: a writer that
reuses a buffer, a file edited by another tool, a field that was overwritten with a shorter
name. Two records with the same name then compare different, and the bug looks like data
corruption rather than like a comparison that compared too much.

Second: calling `strlen` on the field "because the field is almost always padded". Almost
always is the problem. The full field is the one case that has no terminator, and it is the
case a format with well-chosen widths produces constantly.

Third: `field_to_cstr` returning the number of bytes it copied. Then a caller cannot tell a
full copy from a truncated one — both return `cap - 1` — and cannot ask how much room it
needs. The return value's job is to describe the *source*, not the outcome.

## How libc does it

There is no library function for a NUL-padded field, which is why this exercise exists: the
library gives you `strnlen` for the measurement and `memcpy`/`memcmp` for the bytes, and the
convention is yours to maintain. The Linux manual pages are unusually helpful here, because
`string_copying(7)` names the three kinds of data explicitly — *string*, *null-padded
character sequence*, *length-bounded character sequence* — and lists which functions belong to
each. `strncpy` and `stpncpy` are in the null-padded group, and `strnlen` and `strndup` are
the readers; the `str*` functions with no `n` are strings only.

Implementations do take the padding seriously in one place: `strncpy` zero-fills the rest of
the destination, which is what makes the "equal names are equal bytes" property hold for
records a C program writes. That is also why `strncpy` is slow for a short name in a wide
field — it writes every byte — and why it is a field filler rather than a string copier. S03
e02 is that function.
