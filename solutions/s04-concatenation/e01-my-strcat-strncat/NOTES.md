# s04-e01-my-strcat-strncat: notes

## Why it is correct

Both functions begin by walking `dst` to its terminator:

```c
char *p = dst;
while (*p != '\0')
    p++;
```

After that, `p` is where the append starts, and the rest is a copy. `my_strcat` copies until
it has copied the source's terminator:

```c
while ((*p = *src) != '\0') { p++; src++; }
```

The assignment is the test, so the terminator is written before the loop ends: that single
line is why nothing after it needs to add one.

`my_strncat` has two reasons to stop and writes the terminator itself:

```c
for (size_t i = 0; i < n && src[i] != '\0'; i++)
    *p++ = src[i];
*p = '\0';
```

`i < n` comes first, so `src[n]` is never read — the same `&&` ordering as `strnlen` in S01,
and the same reason: the bound is part of the contract, not an optimisation. The terminator is
written unconditionally, which is what makes the function write `n + 1` bytes rather than `n`,
and what makes `n == 0` leave a valid (unchanged) string rather than nothing.

`return dst` in both: the pointer the caller already had, never the advanced `p`.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| an empty destination | the walk ends at once; the append starts at `dst[0]` |
| an empty source | `my_strcat`'s first assignment writes the terminator; `my_strncat`'s loop never runs |
| `n == 0` | `i < n` is false at once, and the terminator is still written |
| `n` larger than the source | `src[i] != '\0'` stops the loop first |
| `n == SIZE_MAX` | `n` is only compared, never incremented |
| a source with no terminator | `i < n` stops the loop before `src[n]` |
| bytes above 0x7F | nothing compares a byte except against `'\0'` |
| bytes after the new terminator | never written: both loops stop at `p` |

## Alternatives

With the rest of the library, and with a length you already have:

```c
memcpy(dst + dlen, src, slen + 1);                 /* if you know both lengths */
stpcpy(stpcpy(dst, a), b);                         /* S03: chaining, one pass each */
snprintf(dst, cap, "%s%s", dst_text, src);         /* S11: bounded, reports truncation */
```

The first is what a careful program does when it is already tracking the destination's length,
and it is the shape of S04 e05's cursor. The second is the `stpcpy` chain of S03 r03 and is
the fastest correct way to build a string from parts when the room has been checked. The third
is the only one of the three that *reports* that the result did not fit.

The Linux `strcat(3)` page gives the equivalence directly:
`strcat(dst, src)` is `stpcpy(strnul(dst), src), dst` — "find the end, then chain-copy". That
is this exercise's two loops, named.

## The usual mistake

Forgetting the terminator in `my_strncat`. The characters are appended and the buffer is no
longer a string, which the harness shows as `abcdef#` against `abcdef\0`. It is the opposite
mistake to `strncpy`'s (S03 e02), which never adds one — the two functions differ here, and
remembering which is which is most of what this exercise is for.

Second: testing `src[i] != '\0'` before `i < n`, which reads one byte past a full field. The
numbers are identical; the harness's exactly-sized source buffers turn it into an ASan report.

Third: returning `p` instead of `dst`. The value is useful — it is where the next append would
start — and it is not what these functions return. (A function that *does* return it is
`stpcpy`, and that is exactly why S03 e01 asks for both.)

Fourth: writing `my_strcat` as a `strlen` plus a copy. It is correct, it is two passes over
`dst` instead of one, and `strlen` is forbidden here for that reason.

## How libc does it

glibc implements `strcat` as `strlen` followed by `memcpy`, and `strncat` as `strlen`,
`strnlen` on the source, `memcpy` and a terminator — all of them vectorised, so the inner work
runs 16 to 64 bytes at a time. The walk to find the destination's end is still a walk: the
asymptotic cost of `strcat` is `O(strlen(dst) + strlen(src))`, and no implementation can do
better, because the function is not told where the end is.

That is the whole content of e03. A loop that calls `strcat` n times over a growing buffer
costs `O(total²)` however fast each individual call is — on this machine, 80,000 four-character
appends take about 0.1 s with `strcat` and 0.0002 s with an end pointer, a factor of 475. The
library cannot fix it; the interface is the problem, and the fix is to stop throwing away the
end pointer you already have.
