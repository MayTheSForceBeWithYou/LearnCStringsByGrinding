# s04-e04-strncat-size: notes

## Why it is correct

```c
if (cap == 0)
    return -1;
have = strlen(dst);
want = strlen(src);
append_n(dst, src, cap - have - 1);
return have + want < cap ? 0 : -1;
```

`cap == 0` is returned first, because the buffer has no byte at all — not even for a
terminator — and because `cap - have - 1` would otherwise wrap. After that test, `dst` is a
string inside `cap` bytes, so `have < cap` and `cap - have - 1` is a legitimate count: the
characters that fit with the terminator reserved.

The return value is computed from the lengths, not from what `strncat` did, because `strncat`
reports nothing. `have + want < cap` is the familiar "a string of `len` characters fits in
`cap` bytes when `len < cap`" with `len` being the length the result wanted.

`label_build` needs no change: it clears the buffer, appends three times, and ORs the
failures together. Each append is now bounded by what is left, so the three calls compose.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the early `return -1`, before any arithmetic |
| `cap == 1` | `cap - have - 1` is 0: `strncat` appends nothing and rewrites the terminator |
| a full destination (`have == cap - 1`) | the same: the bound is 0 |
| an exact fit | `have + want < cap` is true when `have + want == cap - 1` |
| one character too many | the bound stops the copy; the return value is -1 |
| an empty source | the bound is irrelevant; `strncat` writes only the terminator |
| three appends in a row | each recomputes `have`, so each is bounded by what is left |

## Alternatives

```c
snprintf(dst + have, cap - have, "%s", src);        /* S11: same truncation, and a count back */
my_strlcat(dst, src, cap);                          /* e02: one call, reports the wanted length */
p = app_str(p, end, src);                           /* drill r01: an end pointer, no rescanning */
```

All three are better than `strncat` for this job, and for the same reason: they take or return
something the caller can test, instead of leaving the caller to work out whether anything was
dropped. `strncat` is the only one of the four that reports nothing at all.

Note the cost difference too. This fixed version calls `strlen(dst)` on every append, so
building a label out of `k` pieces walks the destination `k` times — the quadratic shape of
e03 in miniature. For three pieces it does not matter; for a loop it does, and the cursor of
e05 is the fix.

## The usual mistake

`sizeof dst` or `cap` as the third argument, which is this exercise. It is attractive because
every *other* bounded function in the library takes the destination's size there
(`strncpy`, `snprintf`, `strlcat`), and because the name has an `n` in it. `strncat` is the
odd one out: its `n` is about the source.

Second: `cap - have` without the `- 1`, which allows exactly one character too many and puts
the terminator on `dst[cap]`. One byte, every time the source is long enough, and the fence
check catches it where a test of the string's contents would not.

Third: computing `cap - have - 1` before the `cap == 0` test. With `cap` 0 that is `SIZE_MAX`,
and `strncat` is then told it may append about eighteen quintillion characters.

Fourth: deciding that `strncat` is unusable and replacing it with `snprintf`. That is a fine
decision in real code — and it is not this exercise, which is about knowing what the argument
means. A programmer who cannot write `cap - strlen(dst) - 1` correctly will also mis-size the
`snprintf`.

## How libc does it

glibc's `strncat` is `strlen` on the destination, `strnlen` on the source bounded by `n`,
`memcpy` of that many bytes, and a terminator — which is exactly the equivalence the Linux
page prints:

```
stpcpy(mempcpy(strnul(dst), src, strnlen(src, ssize)), ""), dst
```

Reading that line answers most questions about the function: it finds the end of `dst`
(`strnul`), it bounds the source and not the destination (`strnlen(src, ssize)`), and it
always writes a terminator (`stpcpy(..., "")`).

The function's reputation is worse than its behaviour: it does exactly what it says, and what
it says is rarely what a caller wants. That is why the man-pages' `string_copying(7)` groups
it under "null-padded character sequences" rather than with the string functions, and why
every modern alternative takes the destination's size instead.
