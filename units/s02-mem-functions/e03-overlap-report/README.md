# s02-e03-overlap-report `[REPORT]`

## Goal

You are handed an AddressSanitizer report of a kind you have not seen yet — no buffer
overflow, no out-of-bounds byte, just two address ranges — and the source it came from.
Answer nine questions about it **before you run, compile or change anything**, including
arithmetic on the two ranges. Afterwards you can read a `memcpy-param-overlap` report, say by
how many bytes the areas overlapped, and name the one-word fix.

## Background

`memcpy`'s parameters are declared `restrict`, which is the caller's promise that the two
regions do not overlap (C17 6.7.3.1, and 7.24.2.1 for the function). Break that promise and
the behaviour is undefined — not "the bytes come out in some order", but undefined, with no
diagnostic required.

What makes this defect special is that **it usually works**. The obvious implementation of
`memcpy` is a forward loop, and a forward loop is correct whenever the destination is below
the source, which is the common direction for in-place edits. So the call does the right thing
on your machine, with your compiler, at your optimisation level, for your sizes — until one of
those changes. The Linux `memcpy(3)` page's CAVEATS section records the time it changed for
everyone at once: glibc 2.13 reversed the copying order on x86-64 for speed, and a pile of
long-standing programs broke.

AddressSanitizer does not wait for that. It replaces `memcpy` with a version that compares the
two ranges before copying and reports an overlap as an error, with the two ranges printed as
half-open intervals. No memory is out of bounds, so there is no "is located N bytes after"
line and no allocation stack: the whole report is the two ranges and the stack of the call.

The fix is to call the function whose contract allows overlap. `memmove` produces the result
"as if" the source had been copied to a temporary first (C17 7.24.2.2), which it achieves by
choosing a direction. It is not meaningfully slower — on both course platforms it is the same
vectorised code with a comparison in front — and the one-word change is the whole repair.

Read **Foundations: overlap, and what `restrict` promises** and **What the tools say** in
`LESSON.md` before you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

The program deletes the first four characters of a 30-character string in place. Addresses
and PIDs differ on every run; the two ranges' *sizes* and their distance apart do not.

```
==1799294==ERROR: AddressSanitizer: memcpy-param-overlap: memory ranges [0x78b393cd0030,0x78b393cd004b) and [0x78b393cd0034, 0x78b393cd004f) overlap
    #0 0x7cb396d29547 in memcpy (/usr/lib/libasan.so.8+0x129547)
    #1 0x5a277c1369f5 in text_delete .../e03-overlap-report/text.c:16
    #2 0x5a277c13671a in main .../e03-overlap-report/main.c:24
```

Where each answer lives:

| Part of the report | What it tells you |
|---|---|
| the `ERROR:` line | the kind — and here, unusually, the whole of the evidence: two half-open ranges `[start, end)` |
| the first range | the **destination**, because `memcpy`'s first parameter is `dst` |
| the second range | the source |
| frame `#0` | ASan's own `memcpy` interceptor, where the check happened |
| frame `#1` | the first line of course code: the call |

The arithmetic is yours. A range `[a, b)` holds `b - a` bytes; the two ranges here have the
same size, because they are the same copy's two ends. Subtract the two start addresses to see
how far apart they are, and from those two numbers work out how many bytes the ranges have in
common. The addresses are hexadecimal: `0x4b - 0x30` is 27, not 21.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `error.kind` | The name AddressSanitizer gives this error, exactly as the report writes it. |
| `range.size` | How many bytes long is each of the two ranges? An integer. |
| `gap.bytes` | How far apart are the two ranges' first addresses? An integer. |
| `overlap.bytes` | How many bytes do the two ranges have in common? An integer. |
| `fault.file` | The source file of the call that made the overlapping copy. The file name only. |
| `fault.line` | The line number of that call. |
| `fix.function` | Which library function should have been called instead? |
| `forward_loop_ok` | For **this** copy, would a plain forward byte-by-byte loop (`dst[i] = src[i]`, `i` rising) have produced the right bytes? `yes` or `no`. |
| `move.bytes` | How many bytes does this deletion have to move? An integer. |

The last two are the interesting pair. `forward_loop_ok` asks about *this* call, not about
`memcpy` in general: look at which of the two ranges starts lower, and work out whether a
rising index would read each byte before overwriting it. `move.bytes` you can get from the
source — `text.c` has a function whose only job is to compute it — and it should agree with
`range.size`. If it does not, one of your two answers is wrong.

## How to write an answer

One value per line after the `=`. Integers in decimal, even though the report prints
addresses in hex. The error kind is written exactly as the report spells it, hyphens and all.
Answers are not case-sensitive. Two examples in the same form, about a different report:

```
# ranges [0x1000,0x1010) and [0x1008, 0x1018)
range.size = 16
overlap.bytes = 8
```

## Constraints

- **Answer before you run anything.** `make check` runs the grader, and the grader tells you
  which keys are wrong, which is feedback; the exercise is the reading and the arithmetic.
- Do not fix the code. (You will write the correct in-place move four times in drill r01.)
- `capture.txt` is the authority for the run. Your own `make asan ARGS=bad` prints the same
  sizes with different addresses.
- Do the hex subtraction by hand or with a calculator, not by running the program.

## What the program checks

With no arguments the program is only the grader: it reads `answers.txt` and reports each key
as `ok`, `WRONG` or `blank`, and never prints the correct value. `move.bytes` it computes by
calling `text_move_count`, which is correct. The other eight are facts about the report and
the source, compared by a hash of what you wrote. `make check` passes only when all nine are
`ok`.

The defective call is reached only with the argument `bad`, so the grading run itself is clean
under the sanitizers.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s02-e03-overlap-report: 0 of 9 answers correct
```

When you are done it ends with `s02-e03-overlap-report: all checks passed`.

Afterwards, run `make run ARGS=bad` — the plain build, no sanitizer. From `capture-run.txt`,
made from exactly this source:

```
before: abcdefghijklmnopqrstuvwxyz0123
after:  efghijklmnopqrstuvwxyz0123 (length 26)
```

The right answer. That is what undefined behaviour looks like most of the time, and it is the
reason the course runs everything under ASan as well.

<details><summary>Hint</summary>

Both ranges are printed as `[start, end)`. Compute each one's length first; the rest is two
subtractions.

</details>
