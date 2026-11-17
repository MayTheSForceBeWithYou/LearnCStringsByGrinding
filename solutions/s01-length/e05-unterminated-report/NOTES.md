# s01-e05-unterminated-report: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `access.kind` | `read` | line 2 of the report: `READ of size 5` |
| `access.size` | `5` | the same line |
| `region.size` | `4` | `0 bytes after 4-byte region` |
| `fault.function` | `report_width` | frame `#1` of the first stack, `main.c:21` |
| `alloc.file` | `dup.c` | the allocation stack's first frame in this exercise's own code |
| `alloc.line` | `16` | the same frame: `field_dup .../dup.c:16`, the `malloc` |
| `defect.function` | `field_dup` | the function that allocated too little and wrote no terminator |
| `size.function` | `field_dup_size` | the other function in `dup.c`, which returns `strnlen(...) + 1` |
| `malloc.bytes` | `5` | four characters and a terminator |

## Why the size is 5 and the region is 4

The block is four bytes, because `field_dup` asked for `strnlen(field, 4)`, which is 4 for a
full field. `strlen` was then handed those four bytes with no terminator in them: it read all
four, then the fifth, which is in ASan's redzone, and was stopped. ASan reports the whole
range the intercepted function touched, so the size is 5 — the block plus the first byte that
was not the block.

So the two numbers together say the thing worth knowing: *the reader needed one more byte than
the allocation provided*. Whenever a heap report shows a read size exactly one larger than the
region size, with a string function on top, this is the defect: a block sized for the text and
not for the terminator.

## Reading order that gets there fastest

1. The `ERROR:` line: `heap-`, so there is an allocation stack to read.
2. `READ` or `WRITE`. A READ means a reader ran off the end; look for a missing terminator. A
   WRITE means a copy that did not fit; look for the size of the destination.
3. `is located N bytes after an M-byte region`: how big the block is, and how far past it the
   access went.
4. The allocation stack, skipping `malloc` and `ds_malloc_at`: the first frame in the
   program's own files is the `malloc` that is probably wrong.
5. Only then the first stack, to see which innocent function tripped over it.

Steps 3 and 4 are the ones a stack report cannot give you, and they are why heap over-reads
are usually faster to diagnose than stack ones.

## Alternatives

Three ways to write `field_dup` correctly, in the order of how much they rely on the reader
knowing what is going on:

```c
/* 1. measure, allocate one more, copy, terminate */
size_t len = strnlen(field, width);
char *out = malloc(len + 1);
...
memcpy(out, field, len);
out[len] = '\0';

/* 2. use the function whose job the size is */
char *out = malloc(field_dup_size(field, width));

/* 3. the library call, after S13 */
char *out = strndup(field, width);
```

`strndup` (POSIX.1-2008, and in this course's build) is the whole function: it copies at most
`width` bytes, stops at a terminator if it finds one, and **always** terminates the result.
It is the right answer in real code, and the reason this exercise writes the copy by hand is
that the defect is invisible until you have seen where the extra byte comes from. S13 r01 is
`strndup` from scratch.

The shape to prefer in a program of your own is 2: one function owns the size, every allocator
calls it, and the `+ 1` exists in exactly one place. The defect in this exercise is possible
only because the size is computed twice, in two functions, and the two disagree.

## The usual mistake

`malloc(len)` for a string of `len` characters. It is the first exercise of the memory course
and it keeps coming back, because the length and the size differ by one byte which nothing in
the expression mentions. Two habits make it rare: write `malloc(len + 1)` and the terminator
assignment as one unit, never apart; and let one named function return the size, as `dup.c`'s
correct half does.

The second mistake is reading the report's `#1` frame as the defect. `report_width` is four
lines and does exactly what it says. If you "fix" it — by bounding the `strlen`, or by
measuring with `strnlen(name, 4)` — the program stops reporting and every other reader of
`field_dup`'s result is still broken, including `printf("%s")`.

The third is answering `dsalloc.c` for `alloc.file`. That frame is the course's allocation
counter, which every `malloc` in course code passes through; it is never the defect. In a
program of your own the equivalent frames are your wrapper functions, and the rule is the
same: read down the allocation stack to the first frame that chose a *number*.

## How libc does it

`strndup` is the library's answer to this exercise, and its implementation is exactly the
three lines above: `strnlen`, `malloc` of one more, `memcpy`, terminate. Its manual page says
so in one sentence, which is a good argument for reading manual pages: "the memory obtained is
... null-terminated", and the size it allocates is "n + 1".

What libc cannot do is help with the *other* half of this defect — a block that is big enough
but whose terminator was never written. No allocator knows what you intend to store. That is
why ASan's redzones matter so much for string code: they turn "the byte after my data happened
to be zero" from a silent success into a report, which is the difference between the plain and
the sanitizer run of this very program.
