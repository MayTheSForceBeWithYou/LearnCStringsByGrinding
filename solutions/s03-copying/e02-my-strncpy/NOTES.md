# s03-e02-my-strncpy: notes

## Why it is correct

Two loops, sharing one index `i`:

1. copy characters while `i < n` **and then** `src[i] != '\0'`;
2. write NULs while `i < n`.

After both, `i == n`: exactly `n` bytes have been written, whatever the source was. If the
first loop ended because of the terminator, the second writes at least one NUL (the field is
a string). If it ended because `i == n`, the second loop does not run and no NUL exists.

The pointer between the loops, `dst + i`, is the first NUL written or `dst + n`: `stpncpy`'s
return value. `my_strncpy` discards it.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `n == 0` | `i < n` is false at once in both loops: nothing read, nothing written |
| source of exactly `n` characters | loop 1 stops on `i == n` before reading `src[n]` |
| unterminated source with `n` readable bytes | the order `i < n && src[i]`: the count is tested first |
| empty source | loop 1 does not run; loop 2 writes `n` NULs |
| the terminator itself | never copied by loop 1; the first padding NUL takes its place |

## Alternatives

```c
size_t len = strnlen(src, n);
memcpy(dst, src, len);
memset(dst + len, 0, n - len);
return dst + len;                  /* stpncpy */
```

This is what the man page gives as the definition, and it is what a real program should
write if it has to write this at all. The exercise forbids it to make you write the loops.

## The usual mistake

"Fixing" it: adding `dst[n - 1] = '\0'`. That is a different function (and a useful one),
but it is not `strncpy`: it destroys the last character of a value that fills the field.

The second most common: `while (src[i] != '\0' && i < n)`, which reads `src[n]`. With a
terminated source nothing visible happens. With a field-to-field copy (an unterminated
source of exactly `n` bytes) it is an over-read, and the harness's exact-size source buffer
turns it into a report.

## How libc does it

As the alternative above: a vectorised `strnlen`, then `memcpy`, then `memset`. The padding
is why `strncpy` into a large buffer is slow: glibc really does zero the remaining
kilobytes.
