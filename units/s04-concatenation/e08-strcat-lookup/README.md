# s04-e08-strcat-lookup `[LOOKUP]`

## Goal

Answer nine questions about `strcat`, `strncat` and `strlcat` **from the manual pages**, and
quote the sentence that supports each. Two of them are the size formulas the pages state
explicitly, which almost nobody has read. Afterwards you can work out, from the page alone,
how large a destination an append needs — and you will have met the sentence that makes
`strncat`'s `n` unambiguous.

## Background

The appending functions are the part of `<string.h>` whose manual pages carry the most useful
arithmetic, and it is arithmetic most programmers reconstruct badly from memory. The Linux
pages state it directly. For `strcat`:

> The programmer is responsible for allocating a destination buffer large enough, that is,
> strlen(dst) + strlen(src) + 1.

and for `strncat`:

> dst must point to a string contained in a buffer that is large enough, that is, the buffer
> size must be at least strlen(dst) + strnlen(src, ssize) + 1.

Read the second one twice. The bound is `strnlen(src, ssize)` — the source, clipped by the
third argument — **plus one**. That `+ 1` is the terminator `strncat` always writes, and it is
why `strncat` can write `n + 1` bytes when `n` characters are appended. It is also why
`strncat(dst, src, sizeof dst)` is an overflow rather than a bound: the formula says the
buffer must hold what is already there *plus* `n` *plus* one.

The pages also give each function as an equivalence in terms of simpler ones, which is worth
reading as a definition:

> strcat(dst, src) ... is equivalent to  stpcpy(strnul(dst), src), dst

"Find the terminator, then chain-copy from there." That is the cost model of e03 in one line.

The third function in this exercise is `strlcat`, which you wrote in e02 and which this course
cannot call. Its page exists on both course platforms even so, and one of the questions is
about which standard it belongs to — a fact worth having, because "it is on my machine" and
"I may use it" are different statements.

The pages you need:

| | Linux (man-pages) | macOS |
|---|---|---|
| `strcat` | `man 3 strcat` (shared with `strcpy`) | `man 3 strcat` |
| `strncat` | `man 3 strncat` | `man 3 strcat` |
| `strlcat` | `man 3 strlcat` (shared with `strlcpy`) | `man 3 strlcpy` |
| the overview | `man 7 string_copying` | not available |

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question | Where to look |
|---|---|---|
| `strcat.returns` | What does `strcat` return: `dst` or something else? | RETURN VALUE |
| `strncat.result` | `strncat(buf, "cdef", 2)` with `buf` holding `"ab"`: what is in `buf` afterwards? | DESCRIPTION |
| `strncat.bytes_written` | How many **bytes** did that call write in all, counting the terminator? | DESCRIPTION |
| `strncat.short_src` | `strncat(buf, "cd", 9)` with `buf` holding `"ab"`: what is in `buf` afterwards? | DESCRIPTION |
| `strcat.need` | The page's formula for `strcat`'s buffer, applied: `dst` holds 3 characters, `src` has 5. What does it give? | DESCRIPTION |
| `strncat.need` | The page's formula for `strncat`'s buffer, applied: `dst` holds 3, `src` has 5, `ssize` is 2. | DESCRIPTION |
| `strncat.max_bytes` | At most how many bytes can `strncat` write in all, with a third argument of 4? | DESCRIPTION |
| `strncat.reports_truncation` | Does `strncat`'s return value tell the caller the append was truncated? | RETURN VALUE |
| `strlcat.in_posix_2008` | Is `strlcat` part of POSIX.1-2008? | STANDARDS |

The two `need` questions are the formulas applied to concrete numbers, so the answers are
integers. Work them out from the sentence, not from what you think the function does; the
difference between the two formulas is exactly the difference between the two functions.

Then, in `ANSWER.md`, under the heading for each key: the page and the section, and the
sentence (or formula) **quoted exactly**. The file ends with two ungraded questions: one about
the `strcat` equivalence quoted above, and one asking you to write the correct third argument
for `strncat` as an expression.

## How to write an answer

One value per line after the `=`. Strings are written with no quotes (`abcd`, not `"abcd"`).
Two examples in the same form, about a function from another unit:

```
# What does strcpy return?  dst | other
strcpy.returns = dst

# strncpy(buf, "ab", 6): are bytes 2..5 set to NUL?  yes | no
strncpy.pads = yes
```

Answers are not case-sensitive.

## Constraints

- Answer from the pages. Do not write a test program, and do not answer from the lesson.
- Every answer in `answers.txt` has a quote in `ANSWER.md`.
- `strlcat` is **not callable** in this course (SPEC section 10), and its page is still on your
  machine. Read it; do not try to link against it.
- If your platform's page words something differently, quote your platform and say so. The
  nine answers are the same on both.

## What the program checks

Four of the answers describe behaviour, so the program makes the call and compares what
happens with what you say the page promises — including the byte count, which it works out
from the string's length before and after. The other five are facts about the documentation;
for those it compares a hash of your answer. Each key is reported as `ok`, `WRONG` or `blank`,
and the correct value is never printed.

`ANSWER.md` is not graded by the program. It is the half of the exercise you grade yourself:
is each quote exact, and does it actually say what your answer says?

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with
`s04-e08-strcat-lookup: 0 of 9 answers correct`. When you are done it ends with
`s04-e08-strcat-lookup: all checks passed`.

<details><summary>Hint</summary>

Both size formulas are in the DESCRIPTION, in the same sentence as the words "large enough".

</details>
