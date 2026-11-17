# s02-e06-mem-predict `[PREDICT]`

## Goal

Twenty-one outcomes to predict before anything runs: the sign of eight `memcmp` calls, the
result of six `memchr` calls, the bytes after three `memset` calls, and the contents of a
buffer after four overlapping `memmove`s. Afterwards you can say what each of the five `mem`
functions does with any arguments, including the arguments people get wrong.

## Background

The five `mem` functions have short contracts and four sharp edges. This exercise is those
four edges, applied.

**`memcmp` promises only a sign.** C17 7.24.4.1: it returns "an integer greater than, equal
to, or less than zero" according to how the first differing byte compares. Not -1 and 1; not
the difference. glibc has returned both over the years. So the answers here are signs, and
code that compares `memcmp(...)` with 1 is broken whatever it prints today.

**`memcmp` compares as `unsigned char`.** So `0x7F` is less than `0x80`, and `0xFF` is
greater than `0x01` — the opposite of what a `signed char` comparison gives on both course
platforms. Nothing about the sign of `char` affects `memcmp`; everything about it affects a
comparison you write by hand.

**`memset`'s value is converted to `unsigned char`.** C17 7.24.6.1. A caller who passes
`0x141` fills with `0x41`; a caller who passes `-1` fills with `0xFF`. Both are specified, and
neither is an error.

**`memmove` is defined for every overlap.** C17 7.24.2.2: the result is as if the source had
been copied to a temporary buffer first. Which means the answer never depends on an
implementation's direction — but working it out *does* depend on your getting the "as if"
right. Predict each of the four `memmove`s by imagining the source copied aside, then copied
back; do not simulate a forward loop.

Read **Foundations** and the five cards in `LESSON.md` before you start.

## The setup

`main.c` holds the calls, in four groups.

**Eight `memcmp` pairs**, each compared over `n` bytes. The answer for each is the **sign**
of the result: `-1`, `0` or `1`.

| Key | Call |
|---|---|
| `cmp01` | `memcmp("abc", "abc", 3)` |
| `cmp02` | `memcmp("abc", "abd", 3)` |
| `cmp03` | `memcmp("abd", "abc", 3)` |
| `cmp04` | `memcmp("abc", "abd", 2)` |
| `cmp05` | `memcmp("a", "a", 0)` |
| `cmp06` | `memcmp("\x7F", "\x80", 1)` |
| `cmp07` | `memcmp("\xFF", "\x01", 1)` |
| `cmp08` | `memcmp("a\0b", "a\0c", 3)` |

**Six `memchr` calls** over one six-byte array, `a b \0 c b d` — note the NUL in the middle
and the repeated `b`. The answer is the **offset** of the byte found, or `-1` for NULL.

| Key | Call |
|---|---|
| `chr01` | `memchr(hay, 'b', 6)` |
| `chr02` | `memchr(hay, 'b', 1)` |
| `chr03` | `memchr(hay, 'b', 0)` |
| `chr04` | `memchr(hay, '\0', 6)` |
| `chr05` | `memchr(hay, 'd', 6)` |
| `chr06` | `memchr(hay, 'z', 6)` |

**Three `memset` calls** into an 8-byte buffer holding `########`. The answer is all eight
bytes in byte notation.

| Key | Call |
|---|---|
| `set01` | `memset(buf, 0x141, 4)` |
| `set02` | `memset(buf, -1, 2)` |
| `set03` | `memset(buf, 'x', 0)` |

**Four `memmove` calls** inside one 8-byte buffer, each starting from `abcdefgh`. The answer
is all eight bytes.

| Key | Call |
|---|---|
| `mov01` | `memmove(buf + 2, buf, 6)` |
| `mov02` | `memmove(buf, buf + 2, 6)` |
| `mov03` | `memmove(buf + 1, buf, 7)` |
| `mov04` | `memmove(buf, buf, 8)` |

## How to write an answer

- **`cmpNN.sign`**: `-1`, `0` or `1`.
- **`chrNN.at`**: the offset from the start of the array as a decimal integer, or `-1` for
  NULL.
- **`setNN.buf`** and **`movNN.buf`**: all eight bytes in byte notation, no spaces — a
  character as itself, a zero byte as `\0`, an unwritten byte as `#`, any other byte as `\xHH`
  with two upper-case hex digits.

Two examples that are not part of the graded set:

| Call | Answer | Why |
|---|---|---|
| `memcmp("ax", "ay", 1)` | `sign = 0` | the difference is past `n` |
| `memset(buf, 'z', 2)` | `buf = zz######` | two bytes written, six untouched |

## Constraints

- Fill in all twenty-one lines before the first run.
- Do not compile anything to find out, and do not add a `printf` to `main.c`. Draw the eight
  boxes for every buffer answer.
- For the `memmove` answers, work from the standard's "as if copied to a temporary" and not
  from a direction. Two of the four give different answers if you simulate a forward loop, and
  both of those answers would be wrong.
- For `memset(buf, 0x141, 4)` and `memset(buf, -1, 2)`, work out the byte value first and
  write it in the notation the answer asks for.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. All twenty-one
answers are computed by making the call, so none of them is a fact about the documentation and
none is hashed. `make check` passes only when all twenty-one are `ok`, in both the plain and
the ASan build. A key you misspelled is listed as `unknown key`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s02-e06-mem-predict: 0 of 21 answers correct
```

When you are done it ends with `s02-e06-mem-predict: all checks passed`.

Afterwards, answer for yourself with no file to fill in: which of the four `memmove` calls
would still be correct if `memcpy` had been called instead, and what would the other ones
print on this machine? (`make asan` on a copy of the program, with `memcpy`, will tell you
what the sanitizer thinks of the question.)

<details><summary>Hint</summary>

For each `memmove`, write the six or seven source bytes out on a separate line first, then
place them at the destination. The bytes that are neither read nor written keep what they had.

</details>
