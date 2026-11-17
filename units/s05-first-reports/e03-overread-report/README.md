# s05-e03-overread-report `[REPORT]`

## Goal

Read a heap report whose bad access is a **read** inside `printf`, and answer ten questions
about it. The point is the distinction the access kind makes: a WRITE overflow means a copy
that did not fit, a READ overflow means a reader that was handed something that is not a
string. Afterwards you can tell those two defects apart from the report's second line and
know which end of the program to go to.

## Background

Every report you have read so far has had a frame list, and in each one the frame to read was
"the first one in your own files". This report has two stacks — the access and the allocation
— and the useful frame is in a *different* one from the useful line.

The shape is the one from S01 e05, now with `printf` on top instead of `strlen`:

- a fixed-width field of six characters, with no terminator, because the format says six
  characters;
- a heap copy of it, allocated at exactly six bytes;
- a `printf("%s")` that reads the six bytes and keeps going.

The read is reported, and the reader is innocent: `printf` is doing what `%s` is specified to
do, which is read until a zero byte. The defect is the allocation, four frames away in the
other stack, which asked for the field's width rather than the field's width plus one.

The rule to take from it: **for a READ overflow in a string function, the line to fix is
never the line the report names.** It is the copy or the allocation that produced bytes that
are not a string. The report helps anyway, because the "allocated by" stack points straight
at it.

Read **Foundations: the four shapes of a report** and **Distinctions** in `LESSON.md` before
you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

```
==2093119==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6c7120ce0016 at pc 0x705122ad0192 bp 0x7ffca1951090 sp 0x7ffca1950808
READ of size 7 at 0x6c7120ce0016 thread T0
    #0 0x705122ad0191  (/usr/lib/libasan.so.8+0xd0191)
    #1 0x705122af1593 in vprintf (/usr/lib/libasan.so.8+0xf1593)
    #2 0x705122af37d5 in printf (/usr/lib/libasan.so.8+0xf37d5)
    #3 0x5ab1db5e268c in main .../e03-overread-report/main.c:28

0x6c7120ce0016 is located 0 bytes after 6-byte region [0x6c7120ce0010,0x6c7120ce0016)
allocated by thread T0 here:
    #0 0x705122b2c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1)
    #1 0x5ab1db5e5429 in ds_malloc_at .../lib/dsalloc.c:216
    #2 0x5ab1db5e2991 in record_code_dup .../e03-overread-report/record.c:21
    #3 0x5ab1db5e261c in main .../e03-overread-report/main.c:25
```

Where each answer lives:

| Part of the report | What it tells you |
|---|---|
| the `ERROR:` line | `heap-`, so there is an allocation stack to read |
| the second line | `READ`, and how far the reader got before it was stopped |
| frames `#0`–`#2` | inside `libasan`: the check, then ASan's `vprintf`, then its `printf` |
| frame `#3` | the first line of course code: the call that read |
| `is located 0 bytes after N-byte region` | the block's size, and that the bad byte is the first one after it |
| the `allocated by` stack | where the block came from: skip `malloc` and `ds_malloc_at` (the course's allocation counter, `docs/AUTHORING-INFRA.md`), and read the first frame in this exercise's files |

Two numbers to combine: the region is 6 bytes and the access is `READ of size 7`, so the
reader got exactly one byte past the end — which is the signature of a missing terminator
rather than a missing bounds check.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `field.width` | How wide is the record's code field? |
| `dup.bytes` | How many bytes should a heap copy of it, as a string, occupy? |
| `error.kind` | The name ASan gives this error, exactly as the report writes it. |
| `access.kind` | `read` or `write`. |
| `access.size` | How many bytes did that one access touch? |
| `region.size` | How many bytes does the heap block hold? |
| `fault.function` | The first function in the access's frame list that belongs to this exercise. |
| `fault.line` | The line number of that frame. |
| `alloc.function` | In the "allocated by" stack, the first function in this exercise outside `lib/`. |
| `alloc.line` | The line number of that frame. |

`dup.bytes` is not in the report: it is what the allocation *should* have been, and the
difference between it and `region.size` is the defect in one number.

## How to write an answer

One value per line after the `=`. Integers in decimal, identifiers as they appear in the
source. Answers are not case-sensitive. Two examples in the same form, about a different
report:

```
# a heap-buffer-overflow READ of size 5 from a block allocated in field.c:40
access.kind = read
alloc.line = 40
```

## Constraints

- **Answer before you run anything.**
- Do not fix the code.
- `capture.txt` is the authority for the run.
- Two of the ten are about the source: `record.c` is thirty lines, and one of its two
  functions is where the block comes from.

## What the program checks

With no arguments the program is only the grader. `field.width` and `dup.bytes` it computes
from `CODE_WIDTH`; the other eight are compared by a hash. Each key is reported as `ok`,
`WRONG` or `blank`, and the correct value is never printed. `make check` passes only when all
ten are `ok`.

The defective path runs only with the argument `bad`, so the grading run is clean.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e03-overread-report: 0 of 10 answers correct
```

When you are done it ends with `s05-e03-overread-report: all checks passed`.

Afterwards, compare this report with e01's, side by side. Same tool, same program structure,
and four differences: the kind (`heap-` against `stack-`), the access kind (READ against
WRITE), where the object is described (an allocation stack against a frame table), and —
the one that matters — whether the line the report names is the line to change.

<details><summary>Hint</summary>

The region is six bytes and the read was seven. One of those numbers is in the source twice.

</details>
