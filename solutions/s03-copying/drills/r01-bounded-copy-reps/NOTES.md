# s03-r01-bounded-copy-reps: notes

## Why it is correct

All four compare the source length with `cap` as `len < cap` / `len >= cap`.

- `bc_loop` walks the whole source, stores a character only while `i + 1 < cap`, and puts the
  terminator at `min(i, cap - 1)`. `i` ends as `strlen(src)`.
- `bc_format` lets `snprintf` do the work; truncation is `n >= cap`, and a negative `n` (an
  error) is treated as truncation too. The negative test comes before the cast.
- `bc_measured` computes `n = min(len, cap - 1)`, copies `n` bytes and terminates.
- `bc_strict` copies `len + 1` bytes or writes one NUL.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 1` | `bc_loop`: `i + 1 < 1` never true; the others: `min(len, 0)` is 0 |
| exact fit | `len < cap` true in all four |
| one too many | `len >= cap` true; the last character is dropped or the copy refused |
| empty source | zero characters, one terminator |
| a multi-byte character cut in half | not handled, on purpose: a byte copy knows nothing about characters (S16) |

## Alternatives

`bc_measured` and `bc_loop` are the same function with different costs; real code uses the
`memcpy` form. `bc_strict` can be written on top of `bc_measured` only if you are willing to
write a truncated value and then erase it, which is the wrong order (see e06's notes).

## The usual mistake

`len <= cap` for "fits". `(size_t)n >= cap` without first testing `n < 0`. In `bc_loop`,
`i < cap - 1`, which is fine here because `cap >= 1` and wrong the day `cap` can be 0.

## How libc does it

`snprintf` with `"%s"` ends up in the same `strlen` + `memcpy` as `bc_measured`, after
parsing the format; that parsing is the entire overhead of `bc_format`.
