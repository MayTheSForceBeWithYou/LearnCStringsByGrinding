# s05-e02-global-overflow-report: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `table.rows` | `12` | `MONTHS` in `table.h` |
| `row.size` | `4` | `sizeof "Jan"`: three characters and a terminator |
| `error.kind` | `global-buffer-overflow` | the `ERROR:` line |
| `access.kind` | `read` | line 2: `READ of size 1` |
| `variable.name` | `months` | the second "is located" line |
| `variable.file` | `table.c` | the same line: `defined in 'table.c:7:19'` |
| `variable.line` | `7` | the same, before the second colon (19 is the column) |
| `variable.size` | `48` | the same line: `of size 48` |
| `fault.function` | `month_label` | frame `#3`, the first frame in this exercise's files |
| `fault.line` | `19` | the same frame |

12 rows × 4 bytes = 48 bytes, which is the size the report gives. When the three numbers
multiply out, you have read the right object.

## Why the access size is 1

`month_name(13)` returns `months[12]`, a pointer just past the table, and `snprintf`'s `%s`
starts reading there looking for a terminator. The very first byte it reads is in the
redzone, so the report says `READ of size 1` — the interceptor checks byte by byte as the
format walks the string, and it is stopped immediately.

Contrast e01, where `strcpy` reported `WRITE of size 13`: there the interceptor knew the whole
range in advance (it is `strlen(src) + 1`) and checked it before copying. The size in a report
is "how much this operation was about to touch", and for a `%s` that is one byte at a time.

## Reading order that gets there fastest

1. The `ERROR:` line: `global-`, so there is no allocation stack and no frame table — just an
   "is located" line naming a variable.
2. `READ` or `WRITE`.
3. The first frame **in your own files**. Here that is `#3`: `#0` is the shadow check, `#1` is
   ASan's `vsnprintf`, `#2` its `snprintf`. The number of library frames varies; the rule does
   not.
4. The "is located" lines — plural. Skip any that name something like `*.LC0`, which is a
   compiler-generated literal. The useful one names a variable you wrote and gives its file,
   line and size.
5. Then read the source at that line and ask what indexed it.

## Alternatives

The defect is an unchecked index, and there are three ways to close it:

```c
const char *month_name(int m)
{
    if (m < 1 || m > MONTHS) return "???";      /* a total function           */
    return months[m - 1];
}

/* or: make the caller prove it */
assert(m >= 1 && m <= MONTHS);                   /* a documented precondition  */

/* or: take the index as an enum, so the type does the checking */
```

The header here documents the precondition ("which must be 1 to 12") and the caller breaks
it, which is the ordinary arrangement: a function with a precondition is correct, and the
program is not. Whether to check inside is a design decision — a check costs a branch and
turns a programming error into a run-time value the caller has to handle — and the one thing
that is *not* a decision is leaving the precondition undocumented.

Note what the course's own harness does: `guard_new(cap)` gives a buffer of exactly `cap`
bytes so that an off-by-one is a report rather than a near miss. The same idea applied to a
global would be to keep the table and the index in one place, which is what a `switch` or a
lookup function with a check gives you.

## The usual mistake

Answering `variable.name` with `*.LC0`, the first "is located" line. That is a compiler-
generated string literal — on this build, the file name `'table.c'` used by some diagnostic —
and the report mentions it only because it is the nearest named global *after* the bad
address. Globals are packed together, so a report often names two or three, and only the one
you can find in your source is useful.

Second: answering `fault.function` with `vsnprintf` or `snprintf`. Both are frames, both are
named (unlike e01's `#0`), and both are inside `libasan`. The path in the frame is the
giveaway: `/usr/lib/libasan.so.8` is not your program.

Third: reading `table.c:7:19` as line 7 column 7, or as line 19. The format is
`file:line:column`, so the declaration is on line 7, starting at column 19.

## How libc does it

Nothing in libc is involved in the check — this is ASan's instrumentation of a global. The
compiler, when `-fsanitize=address` is on, pads every global with a redzone and emits a
registration call at startup that tells the runtime each global's address, size, name, file
and line. That registration is why the report can name `months` at all, and why it can also
name `*.LC0`: every global is registered, including the ones the compiler invented.

The `snprintf` in the frame list is ASan's own, which forwards to glibc's after checking. A
`%s` conversion in glibc's `vfprintf` walks the string with `strlen` to find its length and
then copies it; under ASan both the walk and the copy are checked, and the walk is what
stopped first here.
