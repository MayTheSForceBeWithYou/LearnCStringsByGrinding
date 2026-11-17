# s04-e02-my-strlcat: notes

## Why it is correct

The function measures, decides, then copies:

```c
while (dlen < size && dst[dlen] != '\0') dlen++;    /* a bounded strnlen */
while (src[slen] != '\0') slen++;                   /* all of src, every time */
if (dlen == size)
    return size + slen;                             /* dst is not a string */
```

The first loop is `strnlen(dst, size)` written out: the bound comes first, so `dst[size]` is
never read, and a destination with no terminator leaves `dlen == size`. That is the test the
BSD contract turns into its special case — append nothing, and report `size + slen`, which is
necessarily `>= size`, so the caller's `ret >= size` test still says "did not fit".

The second loop reads all of `src` unconditionally, because the return value needs its length
whether or not anything is copied. That is the function's one real cost.

The copy reserves the terminator's byte in its condition:

```c
for (size_t i = 0; i < slen && dlen + i + 1 < size; i++)
    dst[dlen + i] = src[i];
```

`dlen + i + 1 < size` is "there is room for this character and still a byte for the
terminator". Then the terminator goes at `dlen + min(slen, size - dlen - 1)`, which is inside
the buffer because `dlen < size` was established by the early return. The return value is
`dlen + slen` — the length it tried to create — not the length it stored.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `size == 0` | the first loop never runs, so `dlen == 0 == size`: the early return, with no access to `dst` |
| `dst` with no terminator | the same early return, reporting `size + slen` |
| an empty `src` | `slen` is 0; the copy loop never runs and the terminator is rewritten where it was |
| an exact fit | the copy's condition is true for every `i` up to `slen - 1` |
| one character too many | the condition fails on the last one; the terminator still goes inside |
| nothing fits (`dlen == size - 1`) | `dlen + 0 + 1 < size` is false at once |
| bytes above 0x7F | nothing compares a byte except against `'\0'` |
| the return value | `dlen + slen`, computed before anything was truncated |

## Alternatives

The library call with the same contract, available since S03:

```c
size_t n = (size_t)snprintf(dst + dlen, size - dlen, "%s", src);   /* n is strlen(src) */
```

With `dlen` already known, `snprintf` gives the same bytes and (after adding `dlen`) the same
number, which is why the harness uses it as the oracle. It costs a format-string parse and it
cannot handle an unterminated destination, so it is the right tool when you control the
buffer and the wrong one when you do not.

The other shape worth knowing is the one that does not measure the source at all:

```c
size_t room = size - dlen - 1;
size_t copied = 0;
while (copied < room && src[copied] != '\0') { dst[dlen + copied] = src[copied]; copied++; }
dst[dlen + copied] = '\0';
return src[copied] == '\0' ? dlen + copied : /* needs the rest of src measured */ ...;
```

It copies and measures in one pass, and then has to finish measuring `src` anyway to produce
the return value. There is no version of this function that avoids reading all of `src`, which
is exactly the criticism that has followed `strlcat` since 1999: the interface forces the cost.

## The usual mistake

Using `strlen(dst)` for the first measurement. It is shorter, it is what the ordinary case
wants, and it reads past the end of a destination that has no terminator — which is the case
the whole contract exists to handle. `strnlen(dst, size)`, or the bounded loop above, is the
only correct measurement.

Second: returning the length stored rather than the length wanted. Then truncation is
undetectable (a full buffer and a cut one both return `size - 1`), and the caller cannot ask
how much room it needed.

Third: `dlen + i < size` as the copy's condition, which fills the last byte with a character
and then writes the terminator at `dst[size]`. The harness's exactly-sized destinations turn
that into a fence failure and an ASan `WRITE` report.

Fourth: handling `size == 0` by special-casing it at the top *and* then computing `size - 1`
somewhere below. One early return, placed after the measurement, covers both `size == 0` and
"no terminator" with the same line.

## How libc does it

OpenBSD's original is almost exactly the code above, written with pointers: walk `dst`
bounded by `size`, then copy while there is room, then terminate, then walk the rest of `src`
to finish the count. glibc added `strlcpy` and `strlcat` in 2.38, after twenty-five years of
refusing to, with the maintainers' long-standing objection recorded in the Linux
`strlcat(3)` page's CAVEATS: the functions read the whole source even when truncating, so
using them to avoid overflows turns an O(n) program into an O(n²) one if the data is
attacker-controlled.

The page's recommended alternatives are instructive: `strlcpy`/`strlcat` for "truncation is
fine and the source is short", `snprintf` with its return value checked for formatted output,
and — new in the man-pages — `strtcpy`/`stpecpy` sketches that take an end pointer instead, the
shape this course builds in S04 e05 and drill r01.
