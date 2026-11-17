# s05-e06-fortify-abort `[REPORT]` `[NO-ASAN]`

## Goal

The last of the four tools, and the one you will meet in production rather than in testing:
glibc's own overflow check, compiled in by `-D_FORTIFY_SOURCE=2` and triggered at run time.
Answer eight questions about what it says, what it does not say, and what stopped the
program. Afterwards you can recognise `*** buffer overflow detected ***` on a user's terminal
and know exactly how much it tells you.

## Background

Fortification is a cooperation between the compiler and the C library. When optimisation is on
and `_FORTIFY_SOURCE` is defined, glibc's headers redirect `strcpy`, `strcat`, `sprintf`,
`memcpy` and about twenty others to checking variants — `__strcpy_chk` and friends — which
take an extra argument: the destination's size, as computed by the compiler's
`__builtin_object_size`. If the compiler can see the destination's size, the check happens at
run time with whatever length the source turns out to have.

That combination is what makes it different from everything else in this unit:

| | Needs | Catches | Costs |
|---|---|---|---|
| a compiler warning | both sizes as constants | at build time | nothing |
| AddressSanitizer | a build with `-fsanitize=address` | any access that leaves an object | 2× time, 3× memory |
| `_FORTIFY_SOURCE` | `-O1` or better, and a *known destination size* | overflows of those destinations | almost nothing |

The price of "almost nothing" is that the message is almost nothing too. It names no buffer,
no line, no function; it prints one line and aborts. You get the fact of the overflow and the
core dump, and that is all.

This exercise is built with `EXTRA_CFLAGS = -O2 -D_FORTIFY_SOURCE=2` and `TAG = REPORT
NO-ASAN`. Both halves matter. The course's normal build passes `-U_FORTIFY_SOURCE` so that
every platform runs the same libc entry points (SPEC section 9.1); this exercise turns it
back on deliberately, after that flag, which is why the order in `common.mk` matters. And it
runs without the sanitizers, because ASan would catch the same overflow first and the
question is what glibc says.

Read **What the tools say** in `LESSON.md` before you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make run ARGS=bad
```

```
*** buffer overflow detected ***: terminated
make[1]: *** [../../../common.mk:131: run] Error 134
```

That is the whole report. One line from glibc, and one from `make` saying how the process
ended.

The program builds `"Hello, <name>!"` in a 16-byte buffer with `strcpy` and two `strcat`s.
The buffer's size is visible to the compiler (it is a local array in the same function), and
the name's length is not (it is a parameter), so the compiler emits no warning and the
library check fires at run time.

| Where the answers come from | |
|---|---|
| the message | what kind of check it was, and what it names (count the identifiers in it: there are none) |
| the `Error 134` line | the exit status `make` saw; 134 is 128 + 6, and signal 6 is `SIGABRT` |
| `greet.h` and `greet.c` | the buffer's size, and the arithmetic for the longest name that fits |
| the lesson | what ASan would have called the same overflow |

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `buffer.size` | How many bytes is the buffer the greeting is built in? |
| `fits.longest` | The longest name that fits, in characters, given that the greeting adds `"Hello, "` and `"!"`. |
| `stopped_by` | What stopped the program: the `compiler`, `libc`, or `asan`? |
| `exit.signal` | Which signal ended it? The macro name. |
| `exit.status` | What exit status does the shell report for that? |
| `names.line` | Does the message name the source line of the overflowing call? `yes` or `no`. |
| `names.buffer` | Does it name the buffer? |
| `asan_would_say` | If the program were built with AddressSanitizer instead, which error kind would it report? |

`fits.longest` is arithmetic, not a lookup: the buffer holds 16 bytes, the greeting contributes
a fixed number of characters, and one byte is the terminator.

## How to write an answer

One value per line after the `=`. Integers in decimal, the signal as its macro name
(`SIGSEGV`, `SIGABRT`, ...), the ASan kind as ASan spells it (`stack-buffer-overflow`,
`heap-buffer-overflow`, `global-buffer-overflow`). Answers are not case-sensitive. Two
examples in the same form, about a different program:

```
# a program killed by a segmentation fault
exit.signal = SIGSEGV
exit.status = 139
```

## Constraints

- **Answer before you run anything.**
- Do not fix the code, and do not change the Makefile's `EXTRA_CFLAGS`: the build settings
  are the exercise.
- `capture.txt` is the authority for the run.
- `exit.status` is what a shell reports for a process killed by that signal, not the number
  the signal has.

## What the program checks

With no arguments the program is only the grader. `buffer.size` and `fits.longest` it computes
from `GREET_SIZE`; the other six are compared by a hash. Each key is reported as `ok`, `WRONG`
or `blank`, and the correct value is never printed. `make check` passes only when all eight
are `ok`, and it runs the plain build only, because the Makefile's `TAG` says `NO-ASAN`.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e06-fortify-abort: 0 of 8 answers correct
```

When you are done it ends with `s05-e06-fortify-abort: all checks passed`.

Two experiments afterwards, both one command:

```bash
make run ARGS=bad EXTRA_CFLAGS=-O2            # -O2 without the macro
make run ARGS=bad EXTRA_CFLAGS=-D_FORTIFY_SOURCE=2   # the macro without -O2
```

Neither aborts: the greeting is printed and the program exits 0. Fortification needs **both**
the macro and optimisation, because the destination's size comes from the optimiser's
`__builtin_object_size`. That is worth knowing the next time a debug build behaves differently
from a release build.

<details><summary>Hint</summary>

Count the characters that `greet_print` adds to the name, and remember the terminator.

</details>
