# s01-e04-length-predict `[PREDICT]`

## Goal

Sixteen measurements of five objects: `strlen`, `strnlen` and `sizeof`, predicted on paper
before anything runs. Afterwards you can look at a declaration and a call and say which of the
three numbers you are being told, which is the skill that stops `sizeof` on a pointer and
`strlen` on a field from ever reaching your code.

## Background

Three expressions measure a `char` object and they answer three different questions:

| | What it measures | Needs a terminator | Needs a bound |
|---|---|---|---|
| `sizeof x` | the storage of `x`'s type, at compile time | no | no |
| `strlen(s)` | the bytes before the first NUL | **yes** | no |
| `strnlen(s, n)` | the bytes before the first NUL, or `n` | no | yes |

Four facts do most of the work in this exercise.

- **`strlen` stops at the first NUL**, not at the end of the object. `char e[] = "ab\0cd";` is
  six bytes with text on both sides of a terminator, and `strlen(e)` is 2. The `cd` is really
  there; nothing in `<string.h>` will look at it.
- **An explicit array size wins, and the rest is zero.** `char f[8] = "hi";` is eight bytes of
  which six are zero (C17 6.7.9p21), so `strlen(f)` is 2 and `sizeof f` is 8.
- **`sizeof` on a pointer measures the pointer.** `const char *p = &a[2];` makes `sizeof p` 8
  and `strlen(p)` the length of the text from `a[2]` on. A pointer into the middle of a string
  is a perfectly good string, which is why `strlen(s + k)` is a normal thing to write.
- **A full field has no terminator**, so `strlen` of it is undefined and `strnlen` of it is
  the width. That is the one question in this set whose answer is a word rather than a number.

Read **Foundations** in `LESSON.md` and the `strlen` and `strnlen` cards before you start.

## The setup

`main.c` declares five objects:

```c
char a[] = "hello";                  /* 6 bytes                            */
char e[] = "ab\0cd";                 /* 6 bytes, with a NUL in the middle   */
char f[8] = "hi";                    /* 8 bytes, zero-filled past the text  */
const char *p = &a[2];               /* a pointer into the middle of a      */
char full[4] = {'A', '1', '2', '3'}; /* 4 characters, no terminator         */
```

and then evaluates sixteen expressions over them:

| Keys | Expressions |
|---|---|
| `a.strlen`, `a.sizeof` | `strlen(a)`, `sizeof a` |
| `a.strnlen0`, `a.strnlen3`, `a.strnlen6` | `strnlen(a, 0)`, `strnlen(a, 3)`, `strnlen(a, 6)` |
| `e.strlen`, `e.sizeof`, `e.tail_strlen` | `strlen(e)`, `sizeof e`, `strlen(&e[3])` |
| `f.strlen`, `f.sizeof`, `f.strnlen8` | `strlen(f)`, `sizeof f`, `strnlen(f, 8)` |
| `p.strlen`, `p.sizeof` | `strlen(p)`, `sizeof p` |
| `full.sizeof`, `full.strnlen4` | `sizeof full`, `strnlen(full, 4)` |
| `full.strlen` | `strlen(full)` |

The program evaluates fifteen of them. It does **not** evaluate `strlen(full)`, and that is
the sixteenth question: there is no terminator in those four bytes, so the call has no defined
result and the answer is the word `undefined`. (`full` is written with braces rather than as
`char full[4] = "A123";`, which means the same thing, because both course compilers warn about
the second form — the lesson's **What the tools say** has the warning.)

## How to write an answer

One decimal integer per line, except `full.strlen`, which may be a number or the word
`undefined`. Two examples that are not part of the graded set:

| Expression | Answer | Why |
|---|---|---|
| `strlen("")` | `0` | the empty string is a terminator and nothing else |
| `strnlen("abc", 2)` | `2` | the bound is reached before the terminator, so the answer is the bound |

White space around the `=` does not matter, and `undefined` is not case-sensitive.

## Constraints

- Fill in all sixteen lines before the first run. A prediction made after seeing which lines
  are wrong is a guess with feedback.
- Do not compile anything to find out, and do not add a `printf` to `main.c`. Draw the bytes
  of each object first, one box per byte, with the terminators marked.
- `sizeof` is a pointer's size on both course platforms: 8.
- For every `strnlen`, ask which of its two reasons to stop comes first.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. Fifteen answers
are compared with the result of evaluating the expression; `full.strlen` is compared by a
hash, so `main.c` does not give it away. `make check` passes only when all sixteen are `ok`,
in both the plain and the ASan build. A key you misspelled is listed as `unknown key`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s01-e04-length-predict: 0 of 16 answers correct
```

When you are done it ends with `s01-e04-length-predict: all checks passed`.

Afterwards, answer for yourself with no file to fill in: for which of the five objects is
`sizeof` a safe capacity to pass to a bounded copy, and for which of them is `strlen` a
question you are not allowed to ask?

<details><summary>Hint</summary>

Three of the sixteen are about `a` measured with a bound, and the three answers are not all
different for the same reason.

</details>
