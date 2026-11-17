# s05-e01-stack-overflow-report `[REPORT]`

## Goal

Read a `stack-buffer-overflow` report and answer ten questions about it before running
anything: what was written, how much, which variable, how big it is, where it was declared,
which line did it, and how far past the end the write went. Afterwards you can take an
unfamiliar stack report and find the declaration it is about in under a minute.

## Background

This is the report you will see most often, and it has a part that the heap and global reports
do not: a **stack-object table**. ASan surrounds each local variable with poisoned redzones and
remembers its name, its byte range within the frame and the line it was declared on, so a
stack report ends with something like

```
  This frame has 1 object(s):
    [32, 40) 'tag' (line 25) <== Memory access at offset 40 overflows this variable
```

which names the variable, says it occupies bytes 32 to 39 of the frame (eight bytes), gives
its declaration line, and marks the offset that was touched. Nothing else in the report is
needed to find the declaration.

The other half of reading a report is **which frame is yours**. ASan replaces `strcpy`,
`strlen`, `memcpy` and their relatives with checking versions, so frame `#0` is inside
`libasan` and the first frame in your own code is `#1`. With clang that frame `#0` is named
(`in strcpy`); with gcc it is an address and a library path. Either way, the line to read is
the first one that belongs to the program.

The program here builds a visitor badge. `badge.c` is careful: it measures, it checks, it
truncates. `main.c` copies a name into a small local buffer with `strcpy` first, and that is
the call that overflows — which is the usual arrangement, because the careful code and the
careless call are rarely in the same file.

Read **Foundations: the four shapes of a report** in `LESSON.md` before you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

Addresses, PIDs and build ids differ on every run; nothing you are asked about does.

```
==2091143==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x78f2ca1d0028 at pc 0x7cf2cd2bc4ce bp 0x7fff1663d250 sp 0x7fff1663ca08
WRITE of size 13 at 0x78f2ca1d0028 thread T0
    #0 0x7cf2cd2bc4cd  (/usr/lib/libasan.so.8+0xbc4cd)
    #1 0x603277386511 in copy_into .../e01-stack-overflow-report/main.c:20
    #2 0x603277386673 in main .../e01-stack-overflow-report/main.c:28

Address 0x78f2ca1d0028 is located in stack of thread T0 at offset 40 in frame
    #0 0x603277386524 in main .../e01-stack-overflow-report/main.c:24

  This frame has 1 object(s):
    [32, 40) 'tag' (line 25) <== Memory access at offset 40 overflows this variable
```

Where each answer lives:

| Part of the report | What it tells you |
|---|---|
| the `ERROR:` line | the kind, and therefore which storage and which table to read |
| the second line | `READ` or `WRITE`, and how many bytes that one access touched |
| frame `#0` of the first stack | inside `libasan`: ASan's replacement for the string function |
| frame `#1` | the first line of course code: the call that made the access |
| the `located in stack of thread T0 ... in frame` block | which function's frame the memory belongs to; the line given there is the function's, not the access's |
| the stack-object table | the variable's name, its byte range `[start, end)`, its declaration line, and the offset that overflowed it |

Two numbers in that table are worth combining: the range `[32, 40)` is eight bytes, and the
access was `WRITE of size 13`, so the write ran five bytes past the end. The last question
asks for that number, and it is arithmetic rather than reading.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `access.kind` | Did the bad access read memory or write it? `read` or `write`. |
| `access.size` | How many bytes did that one access touch? An integer, from the report. |
| `buffer.name` | The name of the variable the report blames. |
| `buffer.where` | Which kind of storage: `stack`, `heap` or `global`? |
| `buffer.size` | How many bytes does that variable have? An integer. |
| `decl.line` | The line number where it is declared. |
| `fault.file` | The source file of the line that made the bad access. |
| `fault.line` | The line number of that line. |
| `frame1.function` | The name of the function in frame `#1`. |
| `overflow.bytes` | How many bytes past the end of the buffer were written? |

`access.size` is 13 for a source of twelve characters, because `strcpy` copies the terminator
too. Working out why it is 13 rather than 12 — and rather than 5, the number of bytes that
landed outside — is most of what this exercise teaches.

## How to write an answer

One value per line after the `=`. Integers in decimal, identifiers as they appear in the
source, file names without a path, the one-word answers exactly as the comment next to each
key offers them. Answers are not case-sensitive. Two examples in the same form, about a
different report:

```
# a stack-buffer-overflow READ of size 5 in a function called field_len
access.kind = read
frame1.function = field_len
```

## Constraints

- **Answer before you run anything.** `make check` runs the grader, and the grader tells you
  which keys are wrong, which is feedback; the exercise is the reading.
- Do not fix the code. Nothing here is yours to repair — this unit is six reports.
- `capture.txt` is the authority for the run. Your own `make asan ARGS=bad` prints the same
  facts with different addresses.
- The declaration line is in the report. Do not count lines in the editor and hope.

## What the program checks

With no arguments the program is only the grader: it reads `answers.txt` and reports each key
as `ok`, `WRONG` or `blank`, and never prints the correct value. `buffer.size` it computes by
compiling `sizeof` on the array; the other nine are facts about the report and the source,
compared by a hash of what you wrote. A key you misspelled is listed as `unknown key`.
`make check` passes only when all ten are `ok`.

The overflowing call is reached only with the argument `bad`, so the grading run itself is
clean under the sanitizers.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e01-stack-overflow-report: 0 of 10 answers correct
```

When you are done it ends with `s05-e01-stack-overflow-report: all checks passed`.

Afterwards, run `make run ARGS=bad` — the plain build, no sanitizer. The program writes five
bytes past a local array and, on this machine, prints the name and carries on: the bytes it
overwrote happened not to matter. That is the same defect with no report at all, and it is
what e05 of this unit is about.

<details><summary>Hint</summary>

Five of the ten answers are in the three lines of the stack-object table.

</details>
