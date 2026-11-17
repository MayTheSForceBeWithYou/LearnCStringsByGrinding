# s00-e04-missing-nul: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `access.kind` | `read` | line 2 of the report: `READ of size 5` |
| `access.size` | `5` | the same line |
| `buffer.name` | `id` | the stack-object table: `[32, 36) 'id' (line 17)` |
| `buffer.where` | `stack` | the first line: `stack-buffer-overflow`, confirmed by `located in stack of thread T0` |
| `buffer.size` | `4` | `[32, 36)` is four bytes; the source says `char id[ID_SIZE]` with `ID_SIZE` 4 |
| `fault.file` | `scan.c` | frame `#1`, the first frame in course code |
| `fault.line` | `20` | the same frame: `scan.c:20`, the `return strlen(id);` |
| `decl.line` | `17` | the stack-object table's `(line 17)` |
| `frame1.function` | `entry_id_length` | frame `#1` again |

## Why the size is 5

`strlen` was asked for the length of a four-byte field with no zero byte in it. ASan's
replacement for `strlen` walks the string, finds the terminator at offset 4 — one byte past
the field, in the redzone — and reports the whole range it touched, from offset 0 to offset 4
inclusive: five bytes. So the number is "the field, plus the first byte that was not the
field". Had the byte at offset 4 not been zero either, the walk would have gone further and
the size would have been larger; what stops it is the redzone check, not the data.

This is worth keeping in mind for every interceptor report. The size in a `strlen`,
`strcpy` or `printf("%s")` report is not the size of one machine access; it is how far the
string function got. A `WRITE of size 13` from `strcpy` means thirteen bytes were being
copied, not that one 13-byte instruction exists.

## Reading order that gets there fastest

1. First line: the **kind** tells you the storage (`stack-`, `heap-`, `global-buffer-overflow`)
   and therefore which table at the bottom of the report to read.
2. Second line: **`READ` or `WRITE`**. For strings, a READ overflow almost always means a
   missing terminator, and a WRITE overflow means a copy that did not fit.
3. The first frame **that is your code** (`#1` here, because `#0` is inside libasan). That is
   where the access happened.
4. The storage block at the bottom: for the stack, the object table with names, ranges and
   declaration lines; for the heap, `is located N bytes after an M-byte region` and the
   `allocated by` stack; for a global, `is located 0 bytes after global variable 'x' defined
   in 'file.c:12'`.
5. Then stop reading the report and start reading the source, at the declaration.

## Alternatives

Nothing to implement, but the defect has three different fixes and they are not equivalent:

| Fix | Effect |
|---|---|
| `char id[ID_SIZE + 1]` and write a terminator after the copy | the field becomes a string; `strlen` is then correct |
| keep four bytes and use `strnlen(id, ID_SIZE)` | no string needed; this is S01 e02, and the right answer for a real fixed-width format |
| keep four bytes and print with `%.*s` | the formatted-output version of the same idea (S11) |

The second is what database and protocol code actually does: fields stay fixed width on the
wire, and the bound travels with them instead of a terminator.

## The usual mistake

Answering `scan.c:16` for the faulting line, because that line is printed in the report too —
in the `Address ... is located in stack of thread T0 at offset 36 in frame` block, which
identifies the *frame* by its function's opening line, not the access. The access is in the
frame list at the top. Two different blocks, two different line numbers, and only one of them
is an instruction.

The other is answering `main.c:20` for `fault.file`/`fault.line`, because it is the only
frame in a file the student has already read. Frame `#2` is the caller of the function that
made the access; it is on the path, but it did not touch the byte.

## How libc does it

The `strlen` that ran is not libc's. When a program is linked with AddressSanitizer, ASan's
runtime *interposes* on the memory and string functions: `strlen`, `strcpy`, `memcpy`,
`strcmp` and about forty others resolve to ASan's own versions, which check each byte's shadow
state as they go and then call the real implementation. That is why frame `#0` is inside
`libasan.so` and why the course's build keeps `-fno-builtin` off except where `FORBID` needs
it: with built-in expansion, a short `strlen` can be computed by inline code the interceptor
never sees, and the report would name a different frame or not appear at all.

glibc's real `strlen` reads whole aligned words and would also have stepped past the field —
but it would have reported nothing, because reading inside its own aligned block is something
it is allowed to do and this program is not.
