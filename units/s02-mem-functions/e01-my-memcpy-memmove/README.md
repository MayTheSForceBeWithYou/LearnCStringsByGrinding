# s02-e01-my-memcpy-memmove `[WRITE]` (M)

## Goal

Write the two byte-moving functions of the C library: `my_memcpy`, which copies `n` bytes
between regions that must not overlap, and `my_memmove`, which copies `n` bytes correctly
however they overlap. Afterwards you know what `restrict` is a promise about, why one
function cannot stand in for the other, and why a copy that works perfectly on your machine
can be undefined behaviour.

## Background

`memcpy` and `memmove` are the floor of the whole library: `strcpy`, `strcat`, `strdup`,
`snprintf` and every string builder you will write is one of these two with a length computed
somewhere. They differ from the `str` functions in one way that changes everything — **they
take a count and look at nothing else.** There is no terminator, no character class, no early
exit. `n` bytes move, whatever those bytes are.

The only difference between the two is what the caller is allowed to pass. `memcpy`'s
parameters are declared `restrict`, which is the *caller's* promise that the two regions do
not overlap (C17 6.7.3.1); `memmove`'s are not, and C17 7.24.2.2 defines its result exactly:
the bytes are copied "as if" into a temporary buffer first, so no byte of the source is read
after the write that would have overwritten it.

That "as if" is the whole exercise. A real `memmove` does not use a temporary: it looks at
the two addresses and chooses a direction. Copying forwards, `d[i] = s[i]` with `i` rising, is
correct when the destination is *below* the source, because every byte is read before the
write that would clobber it. Copying backwards is correct when the destination is above.
Choose wrongly and the first bytes you write destroy source bytes you have not read yet, so
the result is a repeating pattern of the first few bytes — a failure that looks like data
corruption and is in fact a direction.

And `memcpy` is not simply "the one without the check". It is the one whose contract lets the
implementation copy in any order, in any sized pieces, with the widest instructions the
processor has. The Linux `memcpy(3)` page's CAVEATS section tells what happened the year glibc
changed that order: a pile of programs that had been calling `memcpy` on overlapping buffers
for years stopped working, because the undefined behaviour they relied on had been benign and
was no longer. That is what undefined behaviour is like — not a crash, a dependency on
something nobody promised.

Read **Foundations** and the `memcpy` and `memmove` cards in `LESSON.md` before you start.

## The API

`mymove.h` declares:

```c
void *my_memcpy(void *restrict dst, const void *restrict src, size_t n);
void *my_memmove(void *dst, const void *src, size_t n);
```

**`my_memcpy(dst, src, n)`** copies `n` bytes from `src` to `dst` and returns `dst`.

- `n` bytes must be readable at `src` and writable at `dst`, and the two regions must **not**
  overlap. The `restrict` qualifiers are that promise; your function does not check it and
  cannot. The harness never passes overlapping regions to it.
- It writes exactly `n` bytes and reads exactly `n` bytes. No byte outside either region is
  touched.
- `n` may be 0, and then nothing is read or written — but both pointers must still be valid.
- There is no error return.

**`my_memmove(dst, src, n)`** does the same copy and is correct for **every** overlap,
including `dst == src`, and returns `dst`.

- The requirements are those of `my_memcpy` except that the regions may overlap in any way.
- The result is as if the `n` bytes at `src` had been copied to a temporary array and then
  from there to `dst`.
- No allocation: you must not actually use a temporary array for `n` bytes. The two pointers
  tell you everything you need.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `mymove.c`, below the marked line.

## Behaviour

An 8-byte buffer holding `abcdefgh`, with every call made on that buffer:

| Call | Buffer afterwards | Why |
|---|---|---|
| `my_memcpy(dst, src, 5)` into a fresh buffer | the five source bytes | the ordinary case |
| `my_memcpy(dst, src, 0)` | untouched | nothing is read or written |
| `my_memmove(buf + 2, buf, 6)` | `ababcdef` | dst above src: the copy must run backwards |
| `my_memmove(buf, buf + 2, 6)` | `cdefghgh` | dst below src: forwards is correct |
| `my_memmove(buf + 1, buf, 7)` | `aabcdefg` | the one-byte overlap, the worst case for a forward loop |
| `my_memmove(buf, buf + 1, 7)` | `bcdefghh` | and the other direction |
| `my_memmove(buf, buf, 8)` | `abcdefgh` | `dst == src`: nothing changes |
| `my_memmove(dst, src, 0)` | untouched | |
| `my_memcpy(dst, src, 5)` where `src` is `a\0b\xFF\x80` | the same five bytes | a NUL is an ordinary byte here |

The third row is the one to work out by hand before writing any code. Copying
`buf + 2 <- buf` forwards writes `a` at index 2 — which is the byte the copy still has to read
for index 4. A forward loop gives `ababab`; the right answer is `ababcdef`.

## Constraints

- **Do not call** `memcpy`, `memmove`, `memset`, `memccpy`, `strcpy`, `stpcpy` or `strncpy`.
  Those are the functions you are writing. The harness inspects `mymove.o`, and your file is
  compiled with `-fno-builtin` so the compiler may not turn your loop into a library call.
- No allocation, and no array of `n` bytes on the stack. `my_memmove` chooses a direction; it
  does not make a copy.
- Read and write through `unsigned char *`. That is the only type you may inspect arbitrary
  object bytes through (C17 6.5p7), and it also keeps the signedness of `char` out of the way.
- Each byte is read once and written once.
- Both functions return `dst`.

## What the harness checks

- **Six fixed copies** for `my_memcpy`: a word, one byte, `n == 0`, bytes including a NUL,
  bytes above 0x7F, and a 36-byte buffer. Each destination is exactly `n` bytes and each
  source is exactly `n` bytes, so one byte too many in either direction is a report. The
  source must be unchanged afterwards.
- **`n == 0` with zero-byte buffers** for both functions: the return value must still be
  `dst`, and nothing may be read or written.
- **`dst == src`** for `my_memmove`: the bytes must be unchanged.
- **The overlap sweep**: for `n` of 1, 8, 24 and 33, every source offset from −32 to +32
  relative to the destination — 65 overlaps per size, 260 in all — compared byte for byte
  against libc's `memmove` on the same input. A mismatch prints the offset, your buffer and
  libc's.
- **Oracle**: 2000 generated buffers of 0 to 48 bytes, with a generated `n`, through both
  functions, compared with libc on all bytes of the destination plus the guard region.
- **Forbidden calls**: `mymove.o` references none of the seven functions listed above.

## Verify

```bash
make check
```

Success ends with `s02-e01-my-memcpy-memmove: all checks passed` and `== check: OK`, with the
forbidden-call check printing `forbid: none of [...] is called`.

A `my_memmove` that always copies forwards fails in the sweep:

```
       my_memmove differs from memmove: n = 8, src = dst -7
       yours: klmnhijklmnhwxyz
       libc:  klmnhijklmnowxyz
FAIL main.c:53: my_memmove agrees with memmove at every overlap from -32 to +32
```

`src = dst -7` means the source starts seven bytes *before* the destination, so the regions
overlap by one byte at the far end, and the forward loop overwrote the last source byte before
reading it. The two lines below show where: one byte in sixteen differs. The sweep reports the
first failing offset only and then stops comparing, so fix the direction and run it again.

In the ASan stage, a `heap-buffer-overflow` means a loop ran past `n` — most often an index
that starts at `n` rather than `n - 1` in the backward loop.

<details><summary>Hint</summary>

Write `my_memmove` first and compare the two pointers before you write anything. `my_memcpy`
is then one of its two branches.

</details>
