# s01-e03-empty-wraps `[BUG]`

## Goal

A two-function module counts characters of one class in a string. Both functions are five
lines, both look right, and one of them reads gigabytes of memory when it is handed the empty
string. Find the defect, fix it, and explain the arithmetic. Afterwards you will never again
write `strlen(s) - 1` without asking what happens when the string is empty — which is the
most common `size_t` defect in C.

## Background

`strlen` returns a `size_t`, and `size_t` is **unsigned**. Unsigned arithmetic in C does not
overflow and does not go negative: it wraps, by definition. C17 6.2.5p9 says a result that
cannot be represented is reduced modulo one plus the largest representable value. So for
`size_t`:

```
(size_t)0 - 1  ==  SIZE_MAX  ==  18446744073709551615     on both course platforms
```

That is not a bug in the language; it is the arithmetic the type is defined to do, and a great
deal of correct code relies on it. It is a bug in any expression that assumed the subtraction
could produce something below zero. `strlen(s) - 1` is exactly that expression, and it is
written constantly: as a loop bound, as the index of the last character, as the length without
the newline.

The loop in this exercise is the canonical form:

```c
for (size_t i = 0; i <= strlen(s) - 1; i++)
```

For a string of five characters the bound is 4 and the loop runs five times, which is right.
For the empty string the bound is `SIZE_MAX` and `i <= SIZE_MAX` is true for every value `i`
can hold, so the loop reads `s[0]`, `s[1]`, `s[2]`, ... until the operating system stops it.
No comparison can save it: there is no `size_t` greater than `SIZE_MAX`.

Two other things to notice while you are here. The condition recomputes `strlen(s)` on every
iteration, so a loop over an `n`-character string does `n` passes over it — the quadratic
shape that S04 e03 makes you wait for. And the correct loop needs no length at all: the
terminator is the stopping condition, which is the whole point of S00.

Read **Foundations: `size_t` and the wrap at zero** in `LESSON.md` before you start.

## Symptom

`make check` dies in the plain build. The first seven cases pass; the eighth does not finish:

```
== plain+dsalloc
make[1]: *** [../../../common.mk:131: run] Error 139
```

139 is 128 + 11, signal 11, `SIGSEGV`: the program was killed for reading memory it does not
own. (`lib/course.h` lists the exit codes; anything of the form 128 + N is a signal.) No
`FAIL` line is printed, because the function never returns to let a check run.

Run the sanitizer stage by itself for the detail:

```bash
make asan
```

From `capture.txt`, produced from exactly this source (addresses and PIDs differ on your run):

```
FAIL main.c:17: count_upper
=================================================================
==1693800==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6fb740ae00f1 at pc 0x60353ea10920 bp 0x7ffef1042af0 sp 0x7ffef1042ae0
READ of size 1 at 0x6fb740ae00f1 thread T0
    #0 0x60353ea1091f in count_upper .../e03-empty-wraps/stats.c:11
    #1 0x60353ea106f4 in one .../e03-empty-wraps/main.c:17
    #2 0x60353ea10823 in main .../e03-empty-wraps/main.c:31

0x6fb740ae00f1 is located 0 bytes after 1-byte region [0x6fb740ae00f0,0x6fb740ae00f1)
allocated by thread T0 here:
```

Three lines carry the information. **`READ`**: nothing was overwritten; something was read
that should not have been. **`0 bytes after 1-byte region`**: the buffer is one byte long —
that is the empty string, a single terminator — and the bad byte is the one immediately after
it. **Frame `#0`** is in `stats.c`, inside the loop, which is where the access is *and* where
the defect is: unusually for this course, the tool points straight at it.

Under ASan the program stops at the first bad byte, so it dies immediately. In the plain build
nothing stops it until it leaves the mapped heap, which is why the two builds fail so
differently from the same defect.

## What the code is supposed to do

`stats.h` is the specification and it is correct.

- `count_upper(s)` returns how many of the characters of `s` are ASCII upper-case letters,
  `'A'` to `'Z'`.
- `count_digits(s)` returns how many are ASCII digits, `'0'` to `'9'`.
- Both borrow `s`, never modify it, read up to and including its terminator and no further,
  and return 0 for the empty string.

| `s` | `count_upper` | `count_digits` |
|---|---|---|
| `"Hello World"` | 2 | 0 |
| `"ABC"` | 3 | 0 |
| `"abc"` | 0 | 0 |
| `"A1b2C3"` | 2 | 3 |
| `"2026-11-16"` | 0 | 8 |
| `"caf\xC3\xA9"` | 0 | 0 |
| `""` | 0 | 0 |

One of the two functions is already written in the shape the other one should have. Compare
them.

## What to hand in

1. **The fix**, in `stats.c`. There is exactly one defect, in one function, and nothing else
   in the exercise is wrong. Do not change `main.c` or `stats.h`, and do not add a special
   case for the empty string: the loop that needs one is the wrong loop.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted, and which function is the correct
     model.
   - *The mechanism*: `strlen("")`, that value minus one, the type it has and the rule that
     gives that result; how many iterations the loop then runs and which byte it reads first
     that it should not.
   - *How the tool showed it*: what the error kind, the access kind and size, and the
     "0 bytes after 1-byte region" line each tell you; why the plain build prints no `FAIL`
     line for this case and what its exit status 139 means.
   - *The rule you take away*: one sentence about subtraction and unsigned types.

## Constraints

- The fixed `count_upper` must still be one pass over the string: no measuring it first.
- Both functions keep their signatures and their contracts.
- No allocation.
- Do not "fix" it by casting to a signed type. `(ptrdiff_t)strlen(s) - 1` happens to work here
  and is a different design: it converts a value whose type was chosen to hold any size into
  one that cannot. Write the loop that does not need the length.

## Verify

```bash
make check
```

Done means `s01-e03-empty-wraps: all checks passed` in the plain stage and again in the ASan
stage, with no report, and `== check: OK`.

Afterwards, in a scratch file outside this directory, try the other two forms of the same
mistake and watch them fail: `buf[strlen(buf) - 1] = '\0'` on an empty buffer (which writes
`buf[-1]`, and is S12 e08), and `for (size_t i = strlen(s) - 1; i >= 0; i--)` to walk a string
backwards (which never ends, because an unsigned value is always `>= 0` — and gcc will tell
you so under `-Wtype-limits`).

<details><summary>Hint</summary>

The other function in the file does not call `strlen` at all.

</details>
