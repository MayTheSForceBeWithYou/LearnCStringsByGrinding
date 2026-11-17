# s04-e06-room-predict `[PREDICT]`

## Goal

Twelve appends into an 8-byte buffer, three answers each: the bytes afterwards, the call's
return value, and the room left. Predict all thirty-six before anything runs. Afterwards you
can look at any of the four appending idioms and say what it writes, what it tells you, and
what is left — which is the difference between an append you have to check and one you can
trust.

## Background

Four ways to append, four different reports:

| Call | What it writes | What it returns | Truncation visible? |
|---|---|---|---|
| `strcat(dst, src)` | all of `src` and a terminator, wherever that lands | `dst` | no — it overflows instead |
| `strncat(dst, src, n)` | at most `n` characters, then always a terminator | `dst` | no |
| `snprintf(dst, cap, "%s%s", ...)` | at most `cap` bytes, always terminated | the length it **wanted** | yes: `ret >= cap` |
| `strlcat`-style | the same bytes as `snprintf` | the length it **wanted** | yes: `ret >= size` |

The two in the middle of that table are the ones people reach for because they have an `n` in
them, and neither of them reports anything. The two at the bottom are the ones that do, and
they report the same number in two types — `int` from `snprintf`, `size_t` from the
`strlcat` contract.

The third answer for each call, the **room left**, is the number a caller needs before the
next append: `cap - strlen(buf) - 1`, the characters that still fit with the terminator
reserved. Writing it out twelve times is how it becomes automatic; drill r02 is the same
arithmetic at the extremes of `size_t`.

One detail about the `snprintf` rows. `snprintf` does not append — it rebuilds — so the calls
in this exercise are written as `snprintf(buf, 8, "%s%s", <old contents>, <src>)`, which
produces the same bytes an append would. That is how `snprintf` is used for appending in real
code when the destination's current contents are known, and it is why S11's formatted output
belongs in the same conversation as `strcat`.

Read **Foundations** and the cards in `LESSON.md` before you start.

## The setup

Each call starts from a buffer whose eight bytes are given exactly — including the `#` bytes
that nothing has written — and appends the string shown.

| Key | Buffer before | Call |
|---|---|---|
| `call01` | `ab\0#####` | `strcat(buf, "cd")` |
| `call02` | `abcdef\0#` | `strcat(buf, "g")` |
| `call03` | `ab\0#####` | `strncat(buf, "cdefgh", 3)` |
| `call04` | `ab\0#####` | `strncat(buf, "cd", 5)` |
| `call05` | `ab\0#####` | `strncat(buf, "cdefgh", 5)` |
| `call06` | `abcdef\0#` | `strncat(buf, "gh", 1)` |
| `call07` | `ab\0#####` | `snprintf(buf, 8, "%s%s", "ab", "cd")` |
| `call08` | `ab\0#####` | `snprintf(buf, 8, "%s%s", "ab", "cdefghij")` |
| `call09` | `abcdefg\0` | `snprintf(buf, 8, "%s%s", "abcdefg", "h")` |
| `call10` | `ab\0#####` | the `strlcat`-style append of `"cd"` |
| `call11` | `ab\0#####` | the `strlcat`-style append of `"cdefghij"` |
| `call12` | `abcdefg\0` | the `strlcat`-style append of `"h"` |

None of the twelve overflows the buffer: every one of them is a legal call. The question is
what each leaves behind and what it tells you.

## How to write an answer

Three lines per call:

- **`callNN.buf`**: all eight bytes in byte notation, no spaces — a character as itself, the
  terminator as `\0`, a byte the call did not write as `#`.
- **`callNN.ret`**: for `strcat` and `strncat`, which return a pointer, the offset of that
  pointer from `buf` (so `0` means "it returned `buf`"); for `snprintf`, the `int`; for the
  `strlcat`-style call, the `size_t`.
- **`callNN.room`**: how many more characters would now fit, `8 - strlen(buf) - 1`.

Two examples that are not part of the graded set:

| Call | `.buf` | `.ret` | `.room` |
|---|---|---|---|
| `strcat(buf, "x")` on `ab\0#####` | `abx\0####` | `0` | `5` |
| `snprintf(buf, 8, "%s%s", "ab", "x")` | `abx\0####` | `3` | `5` |

## Constraints

- Fill in all thirty-six lines before the first run.
- Do not compile anything to find out, and do not add a `printf` to `main.c`. Draw the eight
  boxes for every answer, with the terminator marked.
- For each `strncat` row, count the characters appended *and* the terminator: the two together
  are what the call writes.
- For each `snprintf` and `strlcat` row, the return value describes the **whole** result, not
  what was stored.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. All thirty-six
answers are computed by making the call, so none is a fact about the documentation and none is
hashed. `make check` passes only when all thirty-six are `ok`, in both the plain and the ASan
build.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s04-e06-room-predict: 0 of 36 answers correct
```

When you are done it ends with `s04-e06-room-predict: all checks passed`.

Afterwards, answer for yourself: which of the twelve calls would have overflowed the buffer if
the source had been one character longer, and which would simply have reported a larger
number?

<details><summary>Hint</summary>

Two of the twelve fill the buffer exactly. Find them first; the rest of their rows follow.

</details>
