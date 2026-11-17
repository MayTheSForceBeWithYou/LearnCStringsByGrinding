# s03-e04-strncpy-predict `[PREDICT]`

## Goal

For ten copy calls, write down all eight bytes of the destination buffer and the return value
**before** you run anything. Then let the program grade you. The point is to stop guessing
what `strncpy`, `stpncpy` and `snprintf` leave in a buffer and start knowing it.

## Background

Three functions can copy a string into a buffer with a size, and they disagree about almost
everything:

| | Writes how many bytes | Terminates? | Return value |
|---|---|---|---|
| `strncpy(buf, src, n)` | exactly `n` | only if `strlen(src) < n` | `buf` |
| `stpncpy(buf, src, n)` | exactly `n` | only if `strlen(src) < n` | first NUL written, or `buf + n` |
| `snprintf(buf, n, "%s", src)` | at most `n` | always (if `n > 0`) | `strlen(src)`, as an `int` |

Reading that table is not the same as being able to apply it at 2 a.m. to a buffer in a
debugger. This exercise is the application. The reference cards for all three functions are
in `LESSON.md`; **Worked example 2** does two such predictions in full.

## The setup

`main.c` holds a table of ten calls. Before each call the program fills an 8-byte buffer with
`#`:

```
index:  0 1 2 3 4 5 6 7
buf:    # # # # # # # #
```

Then it makes the call, and compares the buffer and the return value with your prediction in
`answers.txt`.

The ten calls:

| Key | Call |
|---|---|
| `call01` | `strncpy(buf, "hello", 8)` |
| `call02` | `strncpy(buf, "hello", 5)` |
| `call03` | `strncpy(buf, "hello", 3)` |
| `call04` | `strncpy(buf, "", 4)` |
| `call05` | `strncpy(buf, "abcdefgh", 8)` |
| `call06` | `stpncpy(buf, "hey", 8)` |
| `call07` | `stpncpy(buf, "hello", 5)` |
| `call08` | `snprintf(buf, 8, "%s", "hello")` |
| `call09` | `snprintf(buf, 8, "%s", "abcdefghij")` |
| `call10` | `snprintf(buf, 4, "%s", "hello")` |

## How to write an answer

Each call has two lines in `answers.txt`:

- **`callNN.buf`**: all 8 bytes of the buffer after the call, in byte notation, with no
  spaces. A character is written as itself, the terminator as `\0` (backslash, zero), and a
  byte the call did not write as `#`. An answer always describes exactly 8 bytes.
- **`callNN.ret`**: one integer.
  - For `strncpy` and `stpncpy`, which return a pointer: the offset of that pointer from
    `buf`. `0` means "it returned `buf`"; `3` means "it returned `buf + 3`".
  - For `snprintf`: the `int` it returned.

Two examples that are not part of the graded set:

| Call | `.buf` | `.ret` | Why |
|---|---|---|---|
| `strncpy(buf, "hi", 8)` | `hi\0\0\0\0\0\0` | `0` | two characters, then six NULs of padding to make 8 bytes; `strncpy` returns `buf` |
| `snprintf(buf, 8, "%s", "abc")` | `abc\0####` | `3` | three characters and one terminator; bytes 4-7 untouched; returns the length of `"abc"` |

White space around the `=` does not matter. Lines starting with `#` are comments (which is
why a buffer answer must come after `key =` on the same line).

## Constraints

- Fill in all twenty lines before the first run. A prediction made after seeing which lines
  are wrong is a guess with feedback, not a prediction.
- Do not edit `main.c` to print the buffers. If a line is wrong, go back to the function's
  card, work the bytes out on paper with an index row like the one above, and try again.
- No compiler, no debugger, no `man` while predicting. The lesson is allowed.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. `make check`
passes only when all twenty are `ok`, in both the plain and the ASan build. A key you
misspelled is listed as `unknown key`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s03-e04-strncpy-predict: 0 of 20 answers correct
```

When you are done it ends with `s03-e04-strncpy-predict: all checks passed`.

Afterwards, answer for yourself (no file needed): after which of the ten calls is it safe to
pass `buf` to `strlen`? For the others, what would `strlen` read?

<details><summary>Hint</summary>

Count bytes written first, then decide what each one is. For two of the three functions the
count does not depend on the source at all.

</details>
