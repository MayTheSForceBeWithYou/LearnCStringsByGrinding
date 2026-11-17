# s04-e05-append-cursor: notes

## Why it is correct

One helper answers the arithmetic question once:

```c
static size_t room(const Cursor *c)
{
    return c->cap > c->len + 1 ? c->cap - c->len - 1 : 0;
}
```

"How many characters fit at `len`, with the terminator reserved." The comparison is written
`cap > len + 1` rather than `cap - len - 1 > 0` so that nothing is subtracted before it is
known to be safe; `cap == 0` and a full buffer both come out as 0.

`cur_puts` copies `min(strlen(s), room)` bytes with `memcpy`, advances `len`, terminates, and
sets the flag when it copied less than it was given. `cur_putc` is the same with a length of
one. Neither looks at the buffer's contents: the cursor's `len` is the end, which is the whole
point.

`cur_printf` hands `vsnprintf` the room *including* the terminator — `cap - len`, not
`cap - len - 1`, because `vsnprintf` reserves its own — and then reads the return value twice
over:

```c
if (n < 0) { c->buf[c->len] = '\0'; c->truncated = 1; return; }
if ((size_t)n >= c->cap - c->len) { c->len = c->cap - 1; c->truncated = 1; }
else c->len += (size_t)n;
```

A negative result is an encoding error; the cursor repairs the terminator (`vsnprintf` may
have written nothing useful) and records the loss. A result `>= ` the room means truncation,
and the buffer is now full, so `len` becomes `cap - 1`. Otherwise `n` is exactly what was
appended. The order matters: test the negative case *before* converting to `size_t`, or -1
becomes `SIZE_MAX` and the comparison takes the wrong branch for the right reason.

`cap == 0` is returned early in all three appenders, because there is no byte to write a
terminator into and `cur_init` has not written one either.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the early return in each appender; `cur_init` does not touch the buffer |
| `cap == 1` | `room()` is 0, so every append is a loss and the buffer stays `""` |
| an exact fit | `keep == slen` and `keep < slen` is false: no flag |
| one character too many | `keep < slen`: the flag is set, the buffer is full |
| stickiness | the flag is only ever assigned 1, except in `cur_init` |
| an empty append | `slen` is 0, nothing is copied, nothing is lost, no flag |
| `vsnprintf` returning negative | the explicit `n < 0` branch, before any cast |
| a dirty buffer | `cur_init` writes `buf[0] = '\0'` |
| bytes past the terminator | never written: `memcpy` copies `keep` and the terminator follows |

## Alternatives

The same idea with two pointers instead of an index — `p` for "where the next character goes"
and `end` for "one past the buffer" — is drill r01 of this unit, and it is what the Linux
`string_copying(7)` page's `stpecpy` sketch does. Pointers and an index are equivalent; the
index is slightly easier here because `len` is also the number the API reports.

Without a cursor, the same job with the library:

```c
size_t n = strlcat(buf, s, sizeof buf);     /* e02: one call, one comparison */
int k = snprintf(buf + len, cap - len, ...); /* S11: the same, with formatting */
```

Both are correct, both rescan or require the caller to track `len`, and both leave the
truncation check to a caller who may forget it. The cursor's contribution is not the
arithmetic — it is that the check cannot be forgotten, because it happens once at the end.

For a buffer that may grow instead of truncating, the same shape with a `realloc` behind it is
S13's string builder, and `open_memstream` (S13) is the library's version: a `FILE *` that
appends to a heap buffer, so `fprintf` becomes the appender.

## The usual mistake

Passing `cap` rather than `cap - len` to `vsnprintf`. It writes at `buf + len` and is allowed
to use `cap` bytes from there, so the terminator lands up to `len` bytes past the end. The
fences catch it in the plain build and ASan reports a `WRITE` one byte after the region.

Second: clearing the truncated flag on a successful append. The buffer is still missing
characters, and a caller that checks at the end is now told it is not. The flag is a property
of the whole sequence, not of the last call.

Third: treating an exact fit as a truncation, usually by writing the room test with `>=` where
`>` was wanted. Filling a buffer completely is the best case, not the failure case.

Fourth: converting `vsnprintf`'s result to `size_t` before testing it for negativity. `-1`
becomes `SIZE_MAX`, which is `>= ` any room, so the cursor takes the truncation branch — the
right outcome by accident, and wrong the day someone changes the branch.

## How libc does it

`vsnprintf` is the real work here, and its contract is what makes the cursor possible: it
writes at most `size` bytes including the terminator, always terminates when `size > 0`, and
returns the length the complete output would have had. That return value is the only reason a
caller can tell a full buffer from a truncated one — S03's worked example makes the same
point about `snprintf`, and S11 gives the family its cards.

The `va_list` plumbing is the other half. A function that takes `...` cannot pass its
arguments to another variadic function; it has to collect them into a `va_list` with
`va_start` and call the `v`-prefixed variant. That is why the library has `vprintf`,
`vfprintf`, `vsnprintf` and friends at all, and it is why every logging wrapper in every C
program looks like `cur_printf` does.

One caveat worth knowing for later: a `va_list` may be consumed by a single pass, so a wrapper
that needs to use its arguments twice (to measure, then to format) must take a copy with
`va_copy` first. S13 e07 is an exercise in exactly that.
