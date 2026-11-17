# s02-e06-mem-predict: notes

## Why each answer is what it is

**`memcmp`**, sign only, bytes as `unsigned char`:

| Key | Answer | Reason |
|---|---|---|
| `cmp01` | `0` | the three bytes are equal |
| `cmp02` | `-1` | first difference at index 2: `'c'` (99) < `'d'` (100) |
| `cmp03` | `1` | the same pair, the other way round |
| `cmp04` | `0` | only two bytes compared; the difference is at index 2 |
| `cmp05` | `0` | `n == 0`: no bytes compared, so they are equal |
| `cmp06` | `-1` | 0x7F is 127 and 0x80 is 128 **as `unsigned char`** |
| `cmp07` | `1` | 0xFF is 255, greater than 1 |
| `cmp08` | `-1` | the NUL at index 1 does not stop anything; index 2 decides |

**`memchr`** over `a b \0 c b d`:

| Key | Answer | Reason |
|---|---|---|
| `chr01` | `1` | the **first** `b`, not the one at index 4 |
| `chr02` | `-1` | `n == 1` covers only index 0 |
| `chr03` | `-1` | `n == 0`: nothing is read |
| `chr04` | `2` | a NUL byte is found like any other |
| `chr05` | `5` | the search continues past the NUL |
| `chr06` | `-1` | not present |

**`memset`** into `########`:

| Key | Answer | Reason |
|---|---|---|
| `set01` | `AAAA####` | `(unsigned char)0x141` is 0x41, which is `'A'` |
| `set02` | `\xFF\xFF######` | `(unsigned char)-1` is 0xFF |
| `set03` | `########` | `n == 0` writes nothing |

**`memmove`** inside `abcdefgh`:

| Key | Answer | Reason |
|---|---|---|
| `mov01` | `ababcdef` | the six bytes `abcdef` placed at index 2; indexes 0 and 1 keep `ab` |
| `mov02` | `cdefghgh` | the six bytes `cdefgh` placed at index 0; indexes 6 and 7 keep `gh` |
| `mov03` | `aabcdefg` | the seven bytes `abcdefg` placed at index 1; index 0 keeps `a` |
| `mov04` | `abcdefgh` | `dst == src`: every byte is copied onto itself |

## Numbers worth noticing

`cmp06` and `cmp07` are the pair that separates `memcmp` from a hand-written comparison. On
both course platforms plain `char` is signed, so a comparison written as `a[i] - b[i]` with
`char` pointers gives `+1` for `cmp06` and `-1` for `cmp07` — both backwards. Every byte below
0x80 behaves the same either way, which is why this defect survives testing with ASCII data.

`mov01` and `mov03` are the two whose answers a forward loop gets wrong. A forward
`dst[i] = src[i]` for `mov01` writes `a` at index 2, which is a byte it still has to read for
index 4, and produces `ababab##`; for `mov03` it produces `aaaaaaaa`. The right answers come
from the standard's "as if copied to a temporary first" (C17 7.24.2.2), and from an
implementation that copies backwards when the destination is above the source. `mov02` is the
one where a forward loop is correct, which is why the defect in e03 printed the right string.

`set01` and `set02` are the same rule twice: `memset` takes an `int` and uses its low eight
bits. There is no error for a value outside 0..255, and no warning.

## Alternatives

None: these are facts about the five functions. The method that does not slip, for the buffer
answers, is three lines on paper:

```
before:   a  b  c  d  e  f  g  h
source:         c  d  e  f  g  h        (the n bytes at src, written out)
after:    c  d  e  f  g  h  g  h        (source placed at dst; the rest unchanged)
```

Write the source out as its own row *before* placing it, and overlap stops mattering — which
is precisely what "as if copied to a temporary" means.

For the `memcmp` answers, the method is: find the first index where the bytes differ, write
both bytes as numbers from 0 to 255, compare those. For the `memchr` answers: walk from index
0 and stop at the first match or at `n`.

## The usual mistake

Simulating `memmove` with a forward loop, which gives `ababab##` for `mov01`. It is the
natural mental model — the one most people have for "copy" — and it is the model of `memcpy`,
not of `memmove`. The standard's wording exists to make the answer independent of the
implementation's direction.

Second: answering `cmp06` and `cmp07` as if `char` were signed. The bytes are the same in
either case; `memcmp` is specified to interpret them as `unsigned char`, and it is the only
interpretation under which the results are the same on every platform.

Third: `set01 = \x41\x41\x41\x41####`. That is the same four bytes as `AAAA`, and the byte
notation shows printable ASCII as itself — `\xHH` is for the bytes that have no printable
form. The grader compares the rendering, so the notation has to match what `lib/bytes.h`
produces. (Drill r03 of S00 is where that renderer gets written.)

Fourth: forgetting that `n == 0` is legal in all five functions and answering `-1`, "error",
or an unwritten buffer differently from `########`. Zero bytes is a size like any other.

## How libc does it

Each of these functions is vectorised, and the details matter once in this exercise. `memcmp`
is allowed to read all `n` bytes even when it could stop at the first difference, which is
why it is **not** a constant-time comparison and must not be used on secrets. `memchr` turns
the search into a vector compare and a bit scan, so it finds the first match rather than any
match — the specification requires the first, and the implementation is careful to deliver it.

`memmove` chooses a direction, or for small sizes loads the whole source into registers before
storing any of it, which makes overlap irrelevant. `memset` broadcasts the byte into a vector
register. And `memcpy`, for exactly the cases in `mov01`-`mov04`, would have been undefined
behaviour: e03 is a program that called it anyway and got a plausible answer.
