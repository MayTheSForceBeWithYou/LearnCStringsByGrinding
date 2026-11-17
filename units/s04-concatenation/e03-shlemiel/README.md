# s04-e03-shlemiel `[BUG]`

## Goal

A function joins words into a buffer. Every answer it produces is correct, every edge case is
handled, and it fails its cost budget by a factor of several hundred. Find why, fix it without
changing the function's signature or its contract, and explain the arithmetic. Afterwards you
can recognise the quadratic append on sight — it is the most common performance bug in C
string code — and you know the one-line habit that prevents it.

## Background

`strcat(dst, src)` has to find the end of `dst` before it can write anything, and the only way
to find it is to walk from the beginning. The Linux manual page says so in its own terms:
`strcat(dst, src)` "is equivalent to `stpcpy(strnul(dst), src), dst`" — find the terminator,
then copy. That walk is `O(strlen(dst))`, every call.

Put it in a loop that appends `n` pieces and the walks add up:

```
append 1:  walk 0 bytes
append 2:  walk w bytes
append 3:  walk 2w bytes
...
append n:  walk (n-1)w bytes       total ≈ w·n²/2
```

The output is `w·n` bytes, so the useful work is linear and the walking is quadratic. For
small `n` nobody notices. For `n` in the tens of thousands the difference is three orders of
magnitude — the lesson's demo table measures it on this machine, doubling `n` four times and
watching the ratio grow from 47× to 475×.

The name is Joel Spolsky's, from a joke about a painter called Shlemiel who paints the road
from the paint can and walks further back to it every day. The fix is the same as the
painter's: carry the can. The function already computes the length of the result — it has to,
for its return value — so the end position is *already there*; the defect is throwing it away
and asking `strcat` to find it again.

Read **Foundations: what appending costs** in `LESSON.md` before you start.

## Symptom

`make check` fails on a cost check, after passing every correctness check. From
`capture.txt`, produced from exactly this source:

```
FAIL main.c:107: join_words costs at most 50 linear passes (no rescanning the destination)
       reference 0.0004 s, join_words 0.1435 s, budget 0.0233 s
s04-e03-shlemiel: 1 check(s) failed
```

The three numbers are the harness's measurement: the reference is one linear pass that builds
the same string with `memcpy`, timed as the best of three runs; `join_words` is this file;
the budget is fifty times the reference plus five milliseconds of slack. The budget is a
*multiple of a measurement* rather than a number of seconds so that it means the same thing on
a fast laptop and a loaded CI machine.

`make asan` fails the same check with larger numbers on both sides — about 2.5 s against a
budget of 0.09 s. No sanitizer report and no compiler warning: nothing here is out of bounds
or undefined. The program is correct and too slow.

## What the code is supposed to do

`join.h` is the specification and it is correct.

- `join_words(dst, cap, words, n)` joins `n` words with one space between them, no space at
  either end, into a buffer of `cap` bytes, and always terminates when `cap > 0`.
- Truncation is by whole pieces: a word or a separator is appended only if it fits entirely,
  and once something has not fitted nothing after it does either.
- It returns the length of the string it tried to create, like `snprintf`: `>= cap` means
  truncated.

| Call | Result | Returns |
|---|---|---|
| three words, `cap` 32 | `alpha beta gamma` | 16 |
| one word | `alpha` | 5 |
| no words | `` | 0 |
| three words, `cap` 8 | `alpha ` | 16 |
| three words, `cap` 1 | `` | 16 |

All of those pass already. Only the cost check fails.

## What to hand in

1. **The fix**, in `join.c`. There is exactly one defect, in one function, and nothing else in
   the exercise is wrong. Keep the signature, the contract and the return value; do not change
   `main.c` or `join.h`, and do not allocate.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file and the line, the calls quoted, and what each of them must do
     before it can write a byte.
   - *The mechanism*: the total number of bytes read for `n` words of length `w`, as a sum and
     in closed form, then the two numbers for the harness's case (60,000 words of 4
     characters). Say which part of the work is the output and which is the rescanning, and
     why the output is nevertheless correct.
   - *How the tool showed it*: why no sanitizer and no compiler says anything; which check
     failed; what its three numbers mean; and why the budget is a multiple of a measured
     reference rather than a fixed time.
   - *The rule you take away*: one sentence.

## Constraints

- The fixed function must still be one pass over each word and must still produce the same
  string and the same return value for every case above.
- No allocation, no second buffer, and no change to the interface.
- Do not special-case the harness's sizes. The fix is a different shape, not a threshold.
- `strcat` and `strlen` are not forbidden by the Makefile; the budget is the constraint. A
  correct fix will not want them in the loop anyway.

## Verify

```bash
make check
```

Done means `s04-e03-shlemiel: all checks passed` in the plain stage and again in the ASan
stage, and `== check: OK`. A correct version beats the budget by a wide margin: the reference
and your function should be within a small factor of each other, not within fifty.

The budget's shape is worth understanding before you trust it. `50 × reference + 0.005 s` is
deliberately loose: a correct implementation is about 1× the reference, so the check does not
fail on a busy machine, and the quadratic version is 300× or more, so it cannot pass on a fast
one. If you ever write such a check yourself, measure both shapes before choosing the
constant — which is what was done here.

<details><summary>Hint</summary>

The function already knows where the end of the string is. Look at what it returns.

</details>
