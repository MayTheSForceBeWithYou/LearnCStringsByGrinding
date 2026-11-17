# s00-e01-literal-sizes `[PREDICT]`

## Goal

For fourteen declarations, write down what `sizeof` gives and what `strlen` gives **before**
you run anything. Then let the program grade you. Afterwards you can look at any `char`
declaration and say immediately how many bytes of storage it is, how many characters it
holds, and whether asking `strlen` about it is even legal.

## Background

`sizeof` and `strlen` answer two different questions, and almost every buffer bug in C comes
from using the answer to one as the answer to the other.

| | `sizeof x` | `strlen(x)` |
|---|---|---|
| What it is | an operator; the compiler computes it from the *type* of `x` | a function; it runs and counts bytes |
| What it measures | the storage of the object, terminator and padding included | the characters before the first NUL byte |
| Cost | none: a constant in the compiled program | one pass over the bytes |
| On an array of `char` | the array's byte count | the length of the string in it, if there is one |
| On a `char *` | the size of the **pointer** (8 bytes here), whatever it points at | the length of the string it points at |
| If there is no terminator | still correct: it never reads memory | **undefined behaviour**: it reads past the object |

Three facts do most of the work in this exercise:

- A string literal `"abc"` is an array of **four** bytes: the characters and a terminator the
  compiler adds. `sizeof "abc"` is 4.
- `char a[] = "abc";` sizes the array from the initialiser, so `a` is also four bytes. An
  explicit size wins instead: `char c[8] = "abc";` is eight bytes, and the standard requires
  the five bytes after `"abc"` to be zero (C17 6.7.9p21: the rest of an aggregate is
  initialised as if it had static storage duration).
- `char *p = "abc";` declares a *pointer*. The four bytes live somewhere else, in the
  program's read-only data; `p` is just the address of them. `sizeof p` tells you about `p`.

Read **Foundations** in `LESSON.md`, all of it, before you start: the storage table, the
`sizeof` card and the section on escape sequences are exactly this exercise's material.

## The setup

`main.c` declares all fourteen objects, then for each one evaluates `sizeof` and, where the
object is a string, `strlen`, and compares both with your prediction.

| Key prefix | Declaration | Asked about |
|---|---|---|
| `a` | `char a[] = "abc";` | `sizeof a`, `strlen(a)` |
| `p` | `const char *p = "abc";` | `sizeof p`, `strlen(p)` |
| `b` | `char b[3] = {'a', 'b', 'c'};` | `sizeof b`, `strlen(b)` |
| `c` | `char c[8] = "abc";` | `sizeof c`, `strlen(c)` |
| `d` | `char d[] = "ab" "cd";` | `sizeof d`, `strlen(d)` |
| `e` | `char e[] = "a\0b";` | `sizeof e`, `strlen(e)` |
| `f` | `char f[] = "";` | `sizeof f`, `strlen(f)` |
| `g` | `char g[] = {'h', 'i'};` | `sizeof g`, `strlen(g)` |
| `h` | `char h[] = {'h', 'i', '\0'};` | `sizeof h`, `strlen(h)` |
| `m` | `char m[3][8] = {"ab", "cd", "ef"};` | `sizeof m`, `strlen(m[1])` |
| `v` | `const char *v[] = {"a", "bb", "ccc"};` | `sizeof v`, `strlen(v[2])` |
| `q` | `const char *q = &"hello"[2];` | `sizeof q`, `strlen(q)` |
| `r` | `char r[] = "caf\xC3\xA9";` | `sizeof r`, `strlen(r)` |
| `s` | `char s[10] = "hi";` | `sizeof s`, `strlen(s)` |

Two of them, `b` and `g`, are arrays of `char` that hold characters and **no terminator**.
They are not strings. The program therefore never calls `strlen` on them, and the answer for
those two keys is the word `undefined`: not a number, not a guess at what would come out, but
the statement that the standard does not define the behaviour of the call. (`b` is written
with braces rather than as `char b[3] = "abc";` because the second form, which means exactly
the same thing, draws a warning from both course compilers; the lesson's **What the tools
say** has it.)

`q` is written `&"hello"[2]` rather than `"hello" + 2`, which denotes the same address,
because clang rejects the second form under `-Wstring-plus-int`: adding an integer to a
string literal is far more often a mistake than an intention.

Two notes on the environment. A pointer is 8 bytes on both course platforms, and `char` is
exactly 1 byte by definition (C17 6.5.3.4p4: `sizeof (char)` is 1), so every array answer is
just its element count.

## How to write an answer

Each declaration has two lines in `answers.txt`:

- **`<name>.sizeof`**: one decimal integer, the number of bytes.
- **`<name>.strlen`**: one decimal integer, the number of characters before the terminator,
  or the word `undefined` for the two objects that are not strings.

Two examples that are not part of the graded set:

| Declaration | `.sizeof` | `.strlen` | Why |
|---|---|---|---|
| `char w[] = "hi";` | `3` | `2` | the array is sized from the literal: two characters and a terminator |
| `const char *z = "hi";` | `8` | `2` | `sizeof` measures the pointer; `strlen` measures what it points at |

White space around the `=` does not matter, and the word `undefined` is not case-sensitive.

## Constraints

- Fill in all twenty-eight lines before the first run. A prediction made after seeing which
  lines are wrong is a guess with feedback, not a prediction.
- Do not compile anything to find out, and do not add a `printf` to `main.c`. Work each one
  out from the declaration, on paper, writing out the bytes.
- For every array, count the bytes the compiler reserves. For every pointer, ask what
  `sizeof` applies to: the pointer, or the thing it points at.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. The two
`undefined` answers are compared by a hash, so reading `main.c` does not give them away.
`make check` passes only when all twenty-eight are `ok`, in both the plain and the ASan
build. A key you misspelled is listed as `unknown key`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s00-e01-literal-sizes: 0 of 28 answers correct
```

When you are done it ends with `s00-e01-literal-sizes: all checks passed`.

Afterwards, answer for yourself, with no file to fill in: for which of the fourteen is
`sizeof` the right number to pass as a buffer capacity, and for which is it a bug waiting to
happen?

<details><summary>Hint</summary>

Write the bytes out, one box per byte, including the terminator, before writing either
number. Four of the fourteen have a byte the initialiser did not mention.

</details>
