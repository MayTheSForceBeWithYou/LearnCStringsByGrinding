# s01-e05-unterminated-report `[REPORT]`

## Goal

You are handed a real AddressSanitizer report and the source it came from. Answer nine
questions about it **before you run, compile or change anything**: what was accessed, how
much, how big the block was, who allocated it, where, and which of the two functions in the
file is actually wrong. Afterwards you can read a heap over-read back to the allocation that
was one byte too small — the single most common defect in C code that builds strings.

## Background

A heap report has one thing a stack report does not: an **allocation stack**. When ASan
reports an access just outside a heap block, it prints the size of that block and the call
stack of the `malloc` that produced it. Those two facts together are usually enough to find
the defect without reading the program, because the overwhelmingly common cause of a heap
over-read in string code is this:

```
the block was allocated with room for the text, and the terminator went somewhere else
```

The function in this exercise copies a fixed-width field onto the heap as a string. Its
contract says the result is a string — `field_dup("A123", 4)` returns `"A123"` — so the block
must hold five bytes for a four-character field, and the fifth must be written. If either half
is missing, nothing goes wrong at the copy; the program fails later, in whatever innocent
function first treats the result as a string. Here that is a one-line `report_width` that
calls `strlen`, which is exactly where the report points and exactly where the defect is not.

One detail about this course's reports. The allocation stack's first frame is `malloc` inside
ASan's runtime, and its second is `ds_malloc_at` in `lib/dsalloc.c` — the course's allocation
counter, which every `malloc` in course code goes through (`docs/AUTHORING-INFRA.md` explains
the shim). Neither is your code. The first frame that *is* your code is the third, and that is
the one to read.

Read **What the tools say** in `LESSON.md` before you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

Addresses, PIDs and build ids differ on every run; nothing you are asked about does. The part
that matters:

```
==1755113==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x71def5ce0014 at pc 0x75bef7a8841c bp 0x7ffd0e7d9470 sp 0x7ffd0e7d8c28
READ of size 5 at 0x71def5ce0014 thread T0
    #0 0x75bef7a8841b  (/usr/lib/libasan.so.8+0x8841b)
    #1 0x59e8714a54eb in report_width .../e05-unterminated-report/main.c:21
    #2 0x59e8714a55de in main .../e05-unterminated-report/main.c:32

0x71def5ce0014 is located 0 bytes after 4-byte region [0x71def5ce0010,0x71def5ce0014)
allocated by thread T0 here:
    #0 0x75bef7b2c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1)
    #1 0x59e8714a827b in ds_malloc_at .../lib/dsalloc.c:216
    #2 0x59e8714a57d6 in field_dup .../e05-unterminated-report/dup.c:16
    #3 0x59e8714a55bd in main .../e05-unterminated-report/main.c:28
```

Where each answer lives:

| Part of the report | What it tells you |
|---|---|
| the `ERROR:` line | the kind, and therefore that there is an allocation stack to read |
| the second line | `READ` or `WRITE`, and how many bytes that one access touched |
| frame `#0` of the first stack | inside ASan's runtime: it replaces `strlen` and its relatives with checking versions, so this is where the access was detected (with clang the frame is named `in strlen`) |
| frame `#1` of the first stack | the first line of course code: the call that made the access |
| `is located 0 bytes after N-byte region` | how big the block is, and that the bad byte is the first one after it |
| `allocated by thread T0 here` | the stack of the `malloc` that produced the block; skip `malloc` and `ds_malloc_at`, and read the first frame in this exercise's own files |

And the question the report cannot answer: **which line is wrong**. The reader is correct about
what it was told. The allocation is where the mistake is, and the file holds a second function
that already computes the number the allocation should have used — finding it is part of the
exercise.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `access.kind` | Did the bad access read memory or write it? `read` or `write`. |
| `access.size` | How many bytes did that one access touch? An integer, from the report. |
| `region.size` | How many bytes does the heap block hold? An integer, from the report. |
| `fault.function` | The function in frame `#1` of the first stack: the one that made the access. |
| `alloc.file` | The file named in the allocation stack, in the first frame that is this exercise's own code. The file name only. |
| `alloc.line` | The line number in that frame. |
| `defect.function` | The function that contains the defect: the one whose line you would change. |
| `size.function` | The function in the same file that already computes the right size. |
| `malloc.bytes` | How many bytes should have been allocated for this field? An integer. |

`access.size` is worth thinking about rather than guessing: when ASan intercepts a string
function it reports the whole range that function touched, so the number tells you how far the
reader got before it was stopped. Work out from the field's width why it is what it is.

## How to write an answer

One value per line after the `=`. Integers in decimal, identifiers as they appear in the
source, file names without a path, the one-word answers exactly as the comment next to the key
offers them. Answers are not case-sensitive. Two examples in the same form, about a different
report:

```
# a heap-buffer-overflow WRITE of size 1 into a 16-byte block allocated in record.c:48
access.kind = write
region.size = 16
alloc.line = 48
```

## Constraints

- **Answer before you run anything.** `make check` runs the grader, and the grader tells you
  which keys are wrong, which is feedback; the exercise is the reading.
- Do not fix the code. (The same class of defect, to be found and fixed from a failing check,
  is S13 e04.)
- `capture.txt` is the authority for everything about the run. Your own `make asan ARGS=bad`
  will print the same facts with different addresses.
- Two of the nine are about the *source*, not the report: `defect.function` and
  `size.function`. Read `dup.c`; it is twenty lines.

## What the program checks

With no arguments the program is only the grader: it reads `answers.txt` and reports each key
as `ok`, `WRONG` or `blank`, and never prints the correct value. `malloc.bytes` it computes by
calling the function in `dup.c` that is correct, so that answer is a fact about the program.
The other eight are facts about the report and the source, compared by a hash of what you
wrote. A key you misspelled is listed as `unknown key`. `make check` passes only when all nine
are `ok`.

The defective call is reached only with the argument `bad`, which is how `capture.txt` was
made, so the grading run itself is clean under the sanitizers.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s01-e05-unterminated-report: 0 of 9 answers correct
```

When you are done it ends with `s01-e05-unterminated-report: all checks passed`.

To see the report for yourself afterwards, `make asan ARGS=bad`. Then try `make run ARGS=bad`,
with no sanitizer. On the machine this was written on it prints

```
width: 4
```

and exits 0: the byte after the block happened to be zero, so the answer is right by luck.
That is the version of this defect that reaches production, and it is why the course builds
everything twice.

<details><summary>Hint</summary>

Compare the two functions in `dup.c`. One of them already adds the byte the other one forgot.

</details>
