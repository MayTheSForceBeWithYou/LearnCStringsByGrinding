# s04-e03-shlemiel: answer

## The defect

`join.c`, lines 17 and 22, in `join_words`:

```c
            strcat(dst, " ");
...
            strcat(dst, words[i]);
```

Each `strcat` must find `dst`'s terminator before it can write a byte, and the only way to
find it is to walk from `dst[0]`. The function already knows where that terminator is — it is
at offset `need`, the number it is accumulating for its return value — and throws the
knowledge away twice per word.

## The mechanism

For `n` words of `w` characters each, joined with one separator, the string after the `i`th
word is about `i(w + 1)` bytes long. The two `strcat` calls for word `i + 1` each walk that
whole string first, so the total walked is

```
2 · Σ i(w + 1)  for i = 0 .. n-1   =   (w + 1) · n(n - 1)   ≈   (w + 1)·n²
```

bytes, against an output of `(w + 1)n` bytes. The useful work is linear; the walking is
quadratic.

For the harness's case — `n` = 60,000, `w` = 4 — the output is 300,000 bytes and the walking
is about 1.8 × 10¹⁰ byte-equivalents. glibc's `strlen` scans 16 to 64 bytes per step, so this
costs about 0.14 s in the plain build and about 2.5 s under ASan, where every access is
checked. The reference pass over the same data takes 0.0004 s.

The output is correct because `strcat` does exactly what it promises: it is the *number of
times* it is asked to find the end that is the defect, not what it does when it gets there.
That is what makes this class of bug survive every test that checks only results.

## How the tool showed it

No sanitizer report: nothing is out of bounds, nothing is undefined, no memory leaks. No
compiler warning: the calls are perfectly ordinary. A tool that could flag this would have to
know that the loop's cost matters, which is a design question, not a safety one.

What caught it is a check with a **cost budget**:

```
FAIL main.c:107: join_words costs at most 50 linear passes (no rescanning the destination)
       reference 0.0004 s, join_words 0.1435 s, budget 0.0233 s
```

The reference is the same join written as one pass with `memcpy`, timed as the best of three
runs to reduce noise. The budget is `50 × reference + 0.005 s`. The multiple is what makes the
check portable: a fixed number of seconds would fail on a loaded CI machine or pass on a fast
one, while the *ratio* between a linear and a quadratic implementation of the same work is a
property of the algorithm and not of the hardware. The additive slack covers timer resolution
when the reference is very fast.

Note also what the check did **not** do: it ran after the correctness checks, so a failure
here means "right answer, wrong shape", which is exactly the diagnosis.

## The rule you take away

Never let an append search for the end of the buffer: keep the position you already have, and
move it.
