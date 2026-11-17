# s03-r02-fixed-field-reps: notes

## Why it is correct

- `field_put` is `strncpy(field, s, width)`: exactly `width` bytes, characters then NUL
  padding, no terminator when the value fills the field. This is the layout `strncpy` exists
  for. The return value comes from a separate `strlen(s) > width`.
- `field_len` is `strnlen(field, width)`: the number of bytes before the first NUL, never
  looking past `width`. A full field has length `width`.
- `field_get` measures with `strnlen`, copies `min(len, cap - 1)` bytes with `memcpy`, and adds
  the terminator the field may not have.
- `field_eq` compares lengths first, then the bytes with `memcmp`. Neither side is read as a
  string of unknown extent.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| value of exactly `width` characters | `strncpy` writes no NUL; `strnlen` returns `width` |
| value longer than `width` | `strncpy` stops at `width`; `strlen(s) > width` reports the cut |
| short value over a long one | `strncpy`'s padding overwrites the old tail |
| full field vs. a longer string | `field_eq`: lengths differ, so `memcmp` is not reached |
| `field_get` with `cap == 1` | `n = 0`; only the terminator is written |

## Alternatives

`field_eq` as `strncmp(field, s, width) == 0 && strnlen(s, width + 1) <= width`: `strncmp`
stops at `width` characters or at a NUL in both, so it is safe on a field, but it calls a
field equal to any longer string that starts with it unless you add the length test.
`field_get` as `snprintf(dst, cap, "%.*s", (int)width, field)`: the precision bounds the read,
which is the standard way to print a field (S11).

## The usual mistake

`strlen(field)`, `strcmp(field, s)`, `printf("%s", field)`, `strcpy(dst, field)`: every one
reads `field[width]` when the field is full. And in `field_put`, "helpfully" terminating,
which truncates 8-character values to 7.

## How libc does it

`strnlen` is a bounded `memchr` for NUL: `p = memchr(s, 0, n); return p ? p - s : n;`.
