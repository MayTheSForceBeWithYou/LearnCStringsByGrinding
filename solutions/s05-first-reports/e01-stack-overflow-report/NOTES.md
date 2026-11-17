# s05-e01-stack-overflow-report: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `access.kind` | `write` | line 2: `WRITE of size 13` |
| `access.size` | `13` | the same line |
| `buffer.name` | `tag` | the stack-object table: `[32, 40) 'tag' (line 25)` |
| `buffer.where` | `stack` | the `ERROR:` line, confirmed by `located in stack of thread T0` |
| `buffer.size` | `8` | `[32, 40)` is eight bytes; the source says `char tag[8]` |
| `decl.line` | `25` | the table's `(line 25)` |
| `fault.file` | `main.c` | frame `#1`, the first frame in course code |
| `fault.line` | `20` | the same frame: the `strcpy` inside `copy_into` |
| `frame1.function` | `copy_into` | frame `#1` again |
| `overflow.bytes` | `5` | 13 written, 8 of them inside: 13 − 8 |

## Why the size is 13 and the overflow is 5

`"Dodecahedron"` is twelve characters, and `strcpy` copies the terminator too, so the call
writes thirteen bytes. ASan's interceptor checks the whole write before performing it and
reports the full range, which is why the size is 13 rather than 1 (the first byte out of
bounds) or 5 (the bytes that landed outside).

Both numbers are useful and they answer different questions. **13** says how much the *call*
was moving, which points at the source: a twelve-character name. **5** says how much damage
it did, which points at the destination: an eight-byte buffer. The report gives you the first
directly and the second by subtraction from the object table.

For a `strcpy` the arithmetic is always the same: `access.size` is `strlen(src) + 1`, and the
overflow is that minus the buffer's size. If the two numbers do not agree with the source you
are reading, you are reading the wrong `strcpy`.

## Reading order that gets there fastest

1. The `ERROR:` line names the kind: `stack-`, so there will be an object table at the end.
2. Line 2: `READ` or `WRITE`, and the size. A WRITE means a copy that did not fit; a READ
   means something ran off looking for a terminator.
3. The **first frame that is your code** — frame `#1` here, because `#0` is ASan's
   interceptor. That is the call.
4. Skip the `located in stack of thread T0 ... in frame #0` block's line number: it names the
   *function* whose frame it is, not the access. Reading it as the faulting line is the usual
   mistake.
5. The object table: name, `[start, end)`, declaration line, and the overflowing offset.
6. Now go to the declaration and ask whether the buffer is too small or the copy is unbounded.

## Alternatives

Three ways this program could have been written so that the defect could not occur, in
increasing order of how much they tell the caller:

```c
char tag[8];
if (strlen(name) + 1 > sizeof tag) return -1;  /* refuse: S03 e06's copy_strict  */
snprintf(tag, sizeof tag, "%s", name);          /* truncate, and n >= sizeof tag  */
char *tag = strdup(name);                       /* size it to the data (S13)      */
```

The program already contains a function of the first kind — `badge_line` measures, tests and
truncates, and returns the length it needed — which is the point of including it: careful code
and a careless call sit side by side in most real programs, and the tool reports the call.

## The usual mistake

Reading `main.c:24` — the line in the "located in stack" block — as the faulting line. That
block identifies the frame by the line its function opens on, not by the access. The access is
in the frame list at the top, at `main.c:20`. Two blocks, two different line numbers, and only
one of them is an instruction.

Second: answering `access.size` with 12 (the name's length) or 5 (the bytes outside). The
report says 13 and means it: the whole range the intercepted function touched.

Third: answering `frame1.function` with `main`. `main` is frame `#2` — it called the function
that made the access. The distinction matters as soon as the stack is ten frames deep: the
defect is usually in the deepest frame that is still your code, and the fix is often one frame
above it.

## How libc does it

The `strcpy` that ran was not glibc's. When a program is linked with AddressSanitizer, ASan
interposes on about forty string and memory functions; its versions check the shadow memory
for every byte of the range and then call the real implementation. That is why frame `#0` is
inside `libasan.so` and why the reported size is the whole copy rather than a single machine
access.

glibc's own `strcpy` would have copied thirteen bytes with two or three wide stores and
noticed nothing. The five bytes past `tag` are part of `main`'s stack frame — in this program
they are padding ASan inserted, and in a program without a sanitizer they would be another
local, a saved register, or the return address. That last possibility is the entire history of
stack-smashing attacks, and it is why the compiler has `-fstack-protector` and why glibc has
the `_FORTIFY_SOURCE` checks of e06.
