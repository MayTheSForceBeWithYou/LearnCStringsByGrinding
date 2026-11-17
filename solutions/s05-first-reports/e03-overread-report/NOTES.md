# s05-e03-overread-report: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `field.width` | `6` | `CODE_WIDTH` in `record.h` |
| `dup.bytes` | `7` | six characters and a terminator |
| `error.kind` | `heap-buffer-overflow` | the `ERROR:` line |
| `access.kind` | `read` | line 2: `READ of size 7` |
| `access.size` | `7` | the same line |
| `region.size` | `6` | `0 bytes after 6-byte region` |
| `fault.function` | `main` | frame `#3`, the first in this exercise's files |
| `fault.line` | `28` | the same frame: the `printf` |
| `alloc.function` | `record_code_dup` | the allocation stack, skipping `malloc` and `ds_malloc_at` |
| `alloc.line` | `21` | the same frame: the `malloc` |

`region.size` is 6 and `dup.bytes` is 7. That one-byte difference is the whole defect, and it
is visible without reading a single line of the program.

## READ against WRITE, and what each means

The access kind is the most informative single word in a string-related report:

| | What it means | Where to look |
|---|---|---|
| `WRITE` | a copy that did not fit | the destination's size, and the call that chose it |
| `READ` | a reader handed bytes that are not a string | the **writer** that left no terminator |

This report is a READ of exactly one byte more than the block, inside `printf`. The reader is
correct: `%s` is specified to read until a zero byte, and there is no zero byte in those six.
The writer is `record_code_dup`, which allocated the field's width and copied the field's
width — faithful to the field, and wrong for a string.

Fixing the reader would be possible (`printf("%.*s", CODE_WIDTH, code)` prints the field
without needing a terminator) and would leave every other reader of that block broken. Fixing
the allocation fixes all of them at once.

## Reading order that gets there fastest

1. `ERROR:` line: `heap-`, so there is an allocation stack.
2. `READ` or `WRITE`, and the size. Here: a reader, one byte over.
3. `is located N bytes after an M-byte region`: the block's size.
4. The **allocation stack**, skipping `malloc` and the course's `ds_malloc_at`: the first
   frame in the program's own files is the `malloc` that chose the size.
5. Only then the access stack, to see which innocent function tripped over it.

Steps 3 and 4 are what a heap report has and the others do not, and they are why a heap
over-read is usually the quickest of the three to diagnose.

## Alternatives

Three ways `record_code_dup` could be right:

```c
char *out = malloc(CODE_WIDTH + 1);   /* and write out[CODE_WIDTH] = '\0' */
char *out = strndup(r->code, CODE_WIDTH);   /* POSIX: copies and terminates (S13) */
/* or do not make a string at all: keep (pointer, length) and print with %.*s */
```

The second is the one to reach for in real code: `strndup` copies at most `n` bytes, stops at
a terminator if it finds one, and **always** terminates — exactly this function, correctly,
in one call. The third is the shape S19 is about: a field has a length, and a program that
carries the length never needs the terminator.

The course's `lib/guard.h` is built on the same observation. `guard_copy(src, n)` makes a
source of exactly `n` bytes so that reading `src[n]` is a report rather than a lucky success,
which is why every harness in this course that measures a field hands it a block of exactly
its own width.

## The usual mistake

Answering `fault.function` with `printf` or `vprintf`. Both appear in the frame list, both
are named, and both live in `/usr/lib/libasan.so.8`. The path is the tell.

Second: concluding that the defect is at `main.c:28` because that is the line the report
names. It is the line that *read*; the line that made the bytes unreadable is
`record.c:21`. For a READ overflow this is always the case, and the report hands you the
other line in the allocation stack.

Third: answering `alloc.function` with `ds_malloc_at`. That frame is the course's allocation
counter, through which every `malloc` in course code passes (`docs/AUTHORING-INFRA.md`); in a
program of your own the equivalent is whatever allocation wrapper you have. The rule is to
read down the allocation stack to the first frame that chose a *number*.

## How libc does it

The `printf` in the frame list is ASan's interceptor, which checks and then calls glibc's.
glibc's `%s` conversion finds the argument's length with `strlen` and copies that many bytes
into the output buffer — so the over-read happens in the `strlen`, and under ASan the walk is
checked byte by byte, which is why the reported size is 7 and not 6 or 1.

Two details worth carrying. `printf("%.*s", n, s)` gives the conversion a maximum, and glibc
then uses `strnlen` instead — the bounded reader, which never looks past `n` bytes. And
`%s` with a null pointer is undefined (glibc prints `(null)`, which is a courtesy, not a
guarantee); SPEC section 10 lists it among the things this course never relies on.
