# s05-e04-compiler-warnings `[PREDICT]`

## Goal

Ten small functions, each one line of C that a compiler may or may not object to. Decide
which ones gcc warns about at `-O2 -Wall -Wextra -Wpedantic`, and under which flag, **before
compiling anything**. Afterwards you know what a compiler can and cannot see about a string
operation — which is the difference between "no warning, so it is fine" and "no warning,
because the size arrived through a parameter".

## Background

A compiler can diagnose a string bug only when it can see both ends of it: the size of the
destination *and* the length of the source, as constants, in the same translation unit after
inlining. When it can, the diagnostics are excellent and name the defect precisely. When it
cannot — because the buffer is a parameter, or the text comes from `argv`, or the call is
behind a one-line helper — it says nothing at all, and the same defect ships.

That asymmetry is the subject of this exercise, and it is why half of the ten snippets are
silent. Several of them are *the same defect* as a noisy one with the size moved behind a
parameter.

The flags involved, all on by default under `-Wall -Wextra` on gcc:

| Flag | Fires when |
|---|---|
| `-Wstringop-truncation` | a bounded copy is known to cut its source short |
| `-Wstringop-overflow` | a copy is known to write past its destination |
| `-Wstringop-overread` | a read is known to run past its source |
| `-Warray-bounds` | an access is known to be outside an object the compiler can see |
| `-Wsizeof-pointer-memaccess` | a `sizeof` on a pointer is used as a byte count |
| `-Wformat` | a conversion specifier does not match its argument's type |
| `-Wformat-truncation` | a formatted write is known to be cut short |
| `-Wunterminated-string-initialization` | an initialiser fills an array exactly, dropping the terminator |

One of those is never the right answer here, and working out which is part of the exercise.

`snippets.c`, in this directory, holds the ten functions. It is **not** compiled by the
harness — it is not in `SRCS` — and the harness ships no capture of its warnings, so nothing
in the directory gives the answers away.

Read **What the tools say** in `LESSON.md` before you start; it quotes the diagnostics for
four similar snippets from `lesson-demo`, with their flags, so you can see the shapes.

## The snippets

Read `snippets.c`. Each function is numbered in a comment:

| # | What it does |
|---|---|
| 01 | `strncpy(dst, src, 8)` with both pointers as parameters |
| 02 | `strncpy(dst, "abcdefgh", sizeof dst)` into a local `char[8]` |
| 03 | `return sizeof p`, where `p` is a `const char *` parameter |
| 04 | `memset(p, 0, sizeof p)`, where `p` is a `char *` parameter |
| 05 | `printf("%s has %s items\n", name, (const char *)&n)` with an `int n` |
| 06 | `printf("%d\n", n)` with a `size_t n` |
| 07 | `strcpy(small, "a much longer greeting")` into a local `char[8]` |
| 08 | `snprintf(label, sizeof label, "user: %s", name)` into a local `char[8]` |
| 09 | `char code[3] = "abc";` |
| 10 | `strlen(id)` where `id` is a `static const char[4]` with no terminator |

Three of them are worth extra thought before you answer.

**03 and 04** are the same mistake — `sizeof` on a pointer — in two settings. Only one draws
a diagnostic, because only one of them *uses* the number for something the compiler
recognises.

**05** passes the address of an `int` for a `%s`. The cast makes the types match, so there is
nothing for `-Wformat` to compare; whether anything else notices is the question.

**08** is a formatted write into a buffer that may be too small. `-Wformat-truncation` exists
for exactly this — so why might it not fire here?

## How to write an answer

Two kinds of line in `answers.txt`:

- **`sNN.warns`**: `yes` or `no`.
- **`sNN.flag`**: the flag, as the diagnostic prints it in square brackets, **with** its
  leading `-W` and **without** any trailing `=`. gcc prints some flags as `[-Wfoo-bar=]`;
  write `-Wfoo-bar`. There is a `.flag` line only for the snippets that warn.

Two examples that are not part of the graded set:

| Snippet | `.warns` | `.flag` |
|---|---|---|
| a call to `gets()` | `yes` | `-Wdeprecated-declarations` |
| `printf("%s", (char *)0)` | `no` | — |

Answers are not case-sensitive.

## Constraints

- **Predict before compiling.** `make warn` compiles `snippets.c` and prints exactly the
  answers; running it first turns the exercise into typing.
- Answer for **gcc**, at `-O2` with `-Wall -Wextra -Wpedantic`. Optimisation matters: several
  of these diagnostics need the inlining and constant propagation that `-O2` does and `-O0`
  does not.
- One flag per warning snippet. Where gcc prints two diagnostics for one line, the answer is
  the first one it prints.
- Do not change `snippets.c`.

## What the program checks

All sixteen answers are facts about what a compiler does, so all sixteen are compared by a
hash of what you wrote: `main.c` does not contain them. Each key is reported as `ok`, `WRONG`
or `blank`, and the correct value is never printed. `make check` passes only when all sixteen
are `ok`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e04-compiler-warnings: 0 of 16 answers correct
```

When you are done it ends with `s05-e04-compiler-warnings: all checks passed`.

Then, and only then:

```bash
make warn                # gcc, at -O2, on snippets.c
make warn CC=clang       # the same file through clang
```

The second is the more interesting run. clang warns about fewer of the ten, warns about one
of them under a flag gcc does not have, and words every message differently. Neither compiler
is more correct; they have different analyses, and a project that builds with both gets the
union. Note which snippets each one misses, and ask yourself what the two have in common.

<details><summary>Hint</summary>

Five of the ten pass a size or a pointer through a function parameter. Four of those five are
silent.

</details>
