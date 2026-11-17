# s00-e04-missing-nul `[REPORT]`

## Goal

You are handed a real AddressSanitizer report and the source it came from. Answer nine
questions about it **before you run, compile or change anything**: what was accessed, how
much of it, which variable, in which storage, from which line, and where that variable is
declared. Afterwards you can take an unfamiliar ASan report and find the defective
declaration in under a minute, which is the single most useful skill in this course.

## Background

The program reads fixed-width record lines: four characters of identifier, a comma, a count.
`"A123,7"`. Fixed-width fields are everywhere — card formats, protocol headers, database rows
— and they have one property that catches every C programmer once: **a field that is exactly
as wide as its contents has no room for a terminator**, so the bytes in it are not a string,
and the moment anything treats them as one it reads past the field.

Here the field is copied into a local array of exactly four bytes and then measured with
`strlen`. `strlen` is defined to read until it finds a zero byte (C17 7.24.6.3: "computes the
length of the string pointed to by `s`"), and there is no zero byte in those four. So it
reads the fifth byte, which belongs to something else, and keeps going until some unrelated
byte happens to be zero. Three things can happen: a plausible wrong number, a crash, or a
report. Without a sanitizer, the usual outcome is the first, in production, months later.

With ASan the fifth byte is inside a *redzone*: a band of bytes the compiler inserts around
each local variable and fills with a pattern the sanitizer recognises as "not yours". Any
access that lands there is reported with the variable's name, its byte range in the frame, and
the line it was declared on. That is why this report can be read back to a declaration, which
is what you are about to practise.

Read **What the tools say** in `LESSON.md` first; it walks through a report of this shape
line by line.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

Addresses, PIDs and build ids differ on every run; nothing you are asked about does. The part
that matters:

```
==1591082==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x75dec55d0064 at pc 0x79dec868841c bp 0x7ffd1a40e5b0 sp 0x7ffd1a40dd68
READ of size 5 at 0x75dec55d0064 thread T0
    #0 0x79dec868841b  (/usr/lib/libasan.so.8+0x8841b)
    #1 0x5a309ca7d9c9 in entry_id_length .../e04-missing-nul/scan.c:20
    #2 0x5a309ca7d608 in main .../e04-missing-nul/main.c:20

Address 0x75dec55d0064 is located in stack of thread T0 at offset 36 in frame
    #0 0x5a309ca7d928 in entry_id_length .../e04-missing-nul/scan.c:16

  This frame has 1 object(s):
    [32, 36) 'id' (line 17) <== Memory access at offset 36 overflows this variable
```

Where each answer lives:

| Part of the report | What it tells you |
|---|---|
| the `ERROR:` line | the kind of problem, and therefore which storage the buffer is in |
| the second line | `READ` or `WRITE`, and the size of the one access that was caught |
| frame `#0` | inside the sanitizer's own library: ASan replaces `strlen` and its relatives with checking versions, so this is where the access was detected, not where your defect is. (With clang the frame is named, `in strlen`.) |
| frame `#1` | the first line of course code: the call that made the access |
| the `Address ... is located in stack of thread T0` block | which frame the memory belongs to |
| the stack-object table | the variable's name, its byte range `[start, end)` in that frame, the line it is declared on, and which offset was touched |

Note what the report does **not** say: which line put the wrong bytes there. No tool can tell
you that; it is always the line that *wrote* or *declared* the buffer, and finding it is your
job. In this program the writer is correct, and the reader is correct about what it was told;
the defect is that the buffer is one byte too small to hold a string, which the last question
is about.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `access.kind` | Did the bad access read memory or write it? `read` or `write`. |
| `access.size` | How many bytes did that one access touch? An integer, taken from the report. |
| `buffer.name` | The name of the variable the report blames. The identifier only, no quotes. |
| `buffer.where` | Which kind of storage is it in: `stack`, `heap` or `global`? |
| `buffer.size` | How many bytes does that variable have? An integer. |
| `fault.file` | The source file of the line that made the bad access. The file name only. |
| `fault.line` | The line number of that line, in that file. |
| `decl.line` | The line number where that variable is declared. |
| `frame1.function` | The name of the function in frame `#1`. The identifier only. |

Two of them are about reading a number out of the report rather than reasoning: `access.size`
is not 1 and not 4, and it is not a typo. When ASan intercepts a string function it reports
the whole range that function touched, so the size tells you how far past the end the reader
got before it was stopped. Work out from the field's width why that number is what it is.

## How to write an answer

One value per line, after the `=`. Integers in decimal, identifiers as they appear in the
source, the one-word answers exactly as the comment next to the key offers them. Answers are
not case-sensitive. Two examples in the same form, about a different report:

```
# a heap-buffer-overflow WRITE of size 1 inside a function called pad_field
access.kind = write
frame1.function = pad_field
```

## Constraints

- **Answer before you run anything.** `make check` runs the grader, and the grader tells you
  which keys are wrong, which is feedback; the exercise is the reading.
- Do not fix the code. This exercise is about the report. (The same defect, to be found and
  fixed from a failing check rather than a report, is S01 e05.)
- Do not add printing to the program to find a line number. The report has them, and so does
  your editor.
- `capture.txt` is the authority for everything about the run. Your own `make asan ARGS=bad`
  will print the same facts with different addresses.

## What the program checks

With no arguments the program is only the grader: it reads `answers.txt` and reports each key
as `ok`, `WRONG` or `blank`, and never prints the correct value. `buffer.size` it computes by
compiling `sizeof` on the field, so it is a fact about the program rather than about the text.
The other eight are facts about the report and the source and are compared by a hash of what
you wrote, so reading `main.c` does not give them away. A key you misspelled is listed as
`unknown key`. `make check` passes only when all nine are `ok`.

The defective call is reached only with the argument `bad`, which is how `capture.txt` was
made, so the grading run itself is clean under the sanitizers.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s00-e04-missing-nul: 0 of 9 answers correct
```

When you are done it ends with `s00-e04-missing-nul: all checks passed`.

To see the report for yourself afterwards, `make asan ARGS=bad`. To see what the same defect
does with no sanitizer, `make run ARGS=bad`. On the machine this was written on it prints

```
id length: 4
count: 7
```

and exits 0. Four is the right answer by luck: the byte after `id` happened to be zero that
time. Change the compiler, the optimisation level or a line of unrelated code and the luck
changes with it; the read is undefined either way. That contrast is the reason the course
builds everything twice.

<details><summary>Hint</summary>

The stack-object table is the shortest path to five of the nine answers. Read `[32, 36) 'id'
(line 17)` as "the object named `id` occupies bytes 32 up to but not including 36 of this
frame, and it was declared on line 17".

</details>
