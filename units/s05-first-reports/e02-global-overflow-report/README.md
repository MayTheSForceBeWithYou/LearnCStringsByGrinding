# s05-e02-global-overflow-report `[REPORT]`

## Goal

Read a `global-buffer-overflow` report — the third storage class, and the one whose report is
shortest — and answer ten questions about it before running anything. Afterwards you can
recognise all three ASan overflow kinds by their first line alone, and you know which block of
each report names the object.

## Background

A global (or `static`) object lives for the whole run, at an address fixed by the linker, and
ASan surrounds it with redzones just as it does a local or a heap block. When an access lands
in one, the report says `global-buffer-overflow` and ends with a line of the shape

```
0x... is located 0 bytes after global variable 'months' defined in 'table.c:7:19' of size 48
```

That one line is the whole object table: the **name**, the **file and line it is defined on**,
and its **size**. There is no allocation stack, because nothing allocated it, and no frame
table, because it is not in a frame. For finding the declaration, this is the easiest of the
three reports to read.

It has one quirk worth knowing in advance. ASan describes the address relative to the nearest
globals it knows about, and a compiler emits plenty of globals you never wrote — string
literals, in particular, appear under names like `*.LC0`. So a report may print **two or more**
"is located" lines, one of which is about a literal and is useless, and one of which is about
your array. Read them all and pick the one that names something from your source.

The program here is a table of month abbreviations indexed by month number. `month_name(m)`
returns `months[m - 1]`, with no check that `m` is between 1 and 12 — the function's comment
says the caller must ensure it — and the caller asks for month 13.

Read **Foundations: the four shapes of a report** in `LESSON.md` before you start.

## The report

`capture.txt`, in this directory, was produced from exactly these sources with

```bash
make asan ARGS=bad
```

```
==2091973==ERROR: AddressSanitizer: global-buffer-overflow on address 0x5d268f664450 at pc 0x7b4b9c2d0192 bp 0x7ffedc315b90 sp 0x7ffedc315308
READ of size 1 at 0x5d268f664450 thread T0
    #0 0x7b4b9c2d0191  (/usr/lib/libasan.so.8+0xd0191)
    #1 0x7b4b9c2f17ce in vsnprintf (/usr/lib/libasan.so.8+0xf17ce)
    #2 0x7b4b9c2f3c65 in snprintf (/usr/lib/libasan.so.8+0xf3c65)
    #3 0x5d268f65c8e8 in month_label .../e02-global-overflow-report/table.c:19
    #4 0x5d268f65c630 in main .../e02-global-overflow-report/main.c:23

0x5d268f664450 is located 48 bytes before global variable '*.LC0' defined in 'table.c' (0x5d268f664480) of size 8
  '*.LC0' is ascii string 'table.c'
0x5d268f664450 is located 0 bytes after global variable 'months' defined in 'table.c:7:19' of size 48
```

Two things to notice before you answer anything.

**The frame list has three library frames, not one.** `#0` is the shadow-memory check, `#1`
is ASan's `vsnprintf`, `#2` is its `snprintf`: the interceptor called the formatter, which
called the check. The first frame that belongs to this program is `#3`. In the stack report of
e01 there was only one library frame; the number varies with how deep the library call goes,
which is why the rule is "the first frame in your own files" rather than "frame #1".

**There are two "is located" lines.** The first is about `*.LC0`, a string literal the
compiler emitted — the file name, used by an assertion or a debug message. It is 48 bytes
*after* the bad address and tells you nothing. The second names `months`, and it is the one
the questions are about.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question |
|---|---|
| `table.rows` | How many rows does the table have? |
| `row.size` | How many bytes is one row? |
| `error.kind` | The name ASan gives this error, exactly as the report writes it. |
| `access.kind` | `read` or `write`. |
| `variable.name` | The variable named in the "is located" line about this program's own data. |
| `variable.file` | The file it is defined in. |
| `variable.line` | The line it is defined on. |
| `variable.size` | Its size in bytes, as the report gives it. |
| `fault.function` | The first function in the frame list that belongs to this exercise. |
| `fault.line` | The line number of that frame. |

`variable.size` and the two computed answers are a cross-check: a table of `table.rows` rows
of `row.size` bytes should be exactly `variable.size` bytes. If the three do not multiply out,
one of them is wrong.

## How to write an answer

One value per line after the `=`. Integers in decimal, identifiers as they appear in the
source, file names without a path, the error kind with its hyphens. Answers are not
case-sensitive. Two examples in the same form, about a different report:

```
# a global-buffer-overflow WRITE in a function called table_set
error.kind = global-buffer-overflow
fault.function = table_set
```

## Constraints

- **Answer before you run anything.**
- Do not fix the code. (The fix is a bounds check in `month_name`, and the exercise is the
  report.)
- `capture.txt` is the authority for the run.
- Two of the ten are about the source rather than the report: read `table.c`, which is
  twenty lines.

## What the program checks

With no arguments the program is only the grader. `table.rows` and `row.size` it computes by
compiling `MONTHS` and `sizeof "Jan"`; the other eight are compared by a hash of what you
wrote. Each key is reported as `ok`, `WRONG` or `blank`, and the correct value is never
printed. `make check` passes only when all ten are `ok`.

The out-of-range call is reached only with the argument `bad`, so the grading run is clean
under the sanitizers.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s05-e02-global-overflow-report: 0 of 10 answers correct
```

When you are done it ends with `s05-e02-global-overflow-report: all checks passed`.

Afterwards, run `make run ARGS=bad` with no sanitizer. The program prints whatever bytes
follow the table in the data segment, interpreted as a string — on this machine, the start of
another string literal. No crash, no report, and a plausible-looking line of output.

<details><summary>Hint</summary>

Of the two "is located" lines, only one names something you can find in the source.

</details>
