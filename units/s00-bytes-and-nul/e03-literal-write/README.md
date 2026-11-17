# s00-e03-literal-write `[BUG]`

## Goal

A two-file module builds message prefixes such as `"WARNING: "`. One function upper-cases a
string in place, and it is correct. Its caller hands it the wrong kind of memory, and the
program dies with a signal before any check can fail. Find the one defect, fix it in the
caller, and explain what kind of storage a string literal is. Afterwards you can tell, from
a declaration alone, whether the bytes of a string may be written.

## Background

`char *name = "warning";` does two things that look like one. It creates an array of eight
bytes somewhere in the program's static storage and sets `name` to the address of its first
byte. The bytes are the compiler's, not yours: C17 6.4.5p7 says that if the program tries to
modify a string literal, the behaviour is **undefined**. Not "an error", not "unsupported":
undefined, which means the standard stops describing what happens next.

What happens next, on both course platforms, is that the linker has placed those bytes in a
section the operating system maps read-only (`.rodata` on Linux, `__TEXT,__cstring` on
macOS), the processor refuses the store, and the process receives `SIGSEGV`. On an embedded
target with no memory protection the same program would quietly succeed and corrupt data
shared by everything that uses that literal. Both outcomes are "undefined behaviour"; the
loud one is the luckier.

The type system does not help you here, and that is worth knowing. In C a string literal has
type `char[N]`, not `const char[N]`, so assigning one to a `char *` is perfectly legal and
draws no warning by default. (C++ made the type `const`; C did not, to avoid breaking code
written before `const` existed.) The protection you get is a convention: **write
`const char *` for a pointer you do not own the bytes of**, and the compiler will then object
when something tries to write through it.

This exercise is also the first example of a pattern that repeats for the rest of the course:
the tool points at the function that touched the memory, and the defect is in the function
that chose the memory. Read **Foundations: where the bytes live** and **What the tools say**
in `LESSON.md` before you start.

## Symptom

`make check` does not print a single `FAIL` line. The program dies:

```
== plain+dsalloc
make[1]: *** [../../../common.mk:131: run] Error 139
```

139 is 128 + 11, and signal 11 is `SIGSEGV`: the program was killed for touching memory it is
not allowed to touch. (`lib/course.h` lists the exit codes the course uses; anything of the
form 128 + N is a signal.) That is all the plain build tells you. Run the sanitizer stage by
itself for the rest:

```bash
make asan
```

From `capture.txt`, produced from exactly this source (addresses and PIDs differ on your run):

```
AddressSanitizer:DEADLYSIGNAL
=================================================================
==1589159==ERROR: AddressSanitizer: SEGV on unknown address 0x6458682ecf80 (pc 0x6458682e40db bp 0x7ffe2857f400 sp 0x7ffe2857f3e0 T0)
==1589159==The signal is caused by a WRITE memory access.
    #0 0x6458682e40db in upcase .../e03-literal-write/upcase.c:14
    #1 0x6458682e425a in prefix_write .../e03-literal-write/prefix.c:30
    #2 0x6458682e3bbb in prefix_case .../e03-literal-write/main.c:26
    #3 0x6458682e3d7f in main .../e03-literal-write/main.c:46
```

Three lines carry the information. **`SEGV on unknown address`**: unlike a
`heap-buffer-overflow`, this address is not near anything ASan knows about, so there is no
"is located N bytes after" line and no allocation stack. **`The signal is caused by a WRITE
memory access`**: the program was storing, not loading. **Frame `#0`** is the instruction
that stored, in `upcase.c` — and `upcase.c` is correct. Frame `#1` is the line that decided
*which* bytes `upcase` would be given, and that is where the defect is.

The address itself is a hint once you have seen a few: it is a few thousand bytes from the
code addresses in the same report (`pc 0x6458682e40db` against `0x6458682ecf80`), because
both live in the executable image, while heap and stack addresses in this course look like
`0x6e3a65ee0090` and `0x7ffc3c3c0610`.

## What the code is supposed to do

`upcase.h` and `prefix.h` are the specification, and both are correct.

- `upcase(s)` upper-cases the ASCII lower-case letters of the string `s` **in place**, leaving
  every other byte alone, and requires that the bytes of `s` be in storage that may be
  written. It cannot check that and does not try. `upcase.c` implements this correctly: do
  not change either file.
- `prefix_name(level)` returns the level's name in lower case (`"info"`, `"warning"`,
  `"error"`) as a pointer to storage that belongs to the module and lives for the whole run.
  The caller may read it, and must not write it or free it. **Calling anything else in the
  module must not change what this function returns.**
- `prefix_write(out, cap, level)` writes the level's name in capitals, then `": "`, then a
  terminator, into the caller's buffer `out` of `cap` bytes. It returns 0, or -1 if the
  prefix does not fit in `cap` bytes, in which case `out` is not written at all.

| Call | Buffer (10 bytes) afterwards | Returns |
|---|---|---|
| `prefix_write(out, 10, PREFIX_WARN)` | `WARNING: \0` | `0` |
| `prefix_write(out, 10, PREFIX_INFO)` | `INFO: \0###` | `0` |
| `prefix_write(out, 10, PREFIX_ERROR)` | `ERROR: \0##` | `0` |
| `prefix_write(out, 8, PREFIX_WARN)` | `########` (untouched) | `-1` |
| `prefix_write(out, 0, PREFIX_WARN)` | untouched | `-1` |
| `prefix_name(PREFIX_WARN)` after any of the above | — | `"warning"`, lower case, every time |

## What to hand in

1. **The fix**, in `prefix.c`. There is exactly one defect, in one function, and nothing else
   in the exercise is wrong. Do not change `main.c`, `prefix.h`, `upcase.h` or `upcase.c`:
   `upcase` is the innocent party, and a "fix" that makes it copy its argument or check for
   read-only memory is a different function from the one its header documents.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted, and which file is innocent.
   - *The mechanism*: where the bytes of `"warning"` live, who put them there, and what the
     store to them runs into.
   - *How the tool showed it*: the signal, the access kind, what frames `#0` and `#1` each
     tell you, why the plain build says only `Error 139`, and which check in `main.c` would
     have caught the defect even on a machine where the write had succeeded.
   - *The rule you take away*: one sentence.

## Constraints

- The level names stay lower case in the module's own storage: after your fix,
  `prefix_name(PREFIX_WARN)` is still `"warning"`.
- `prefix_write` still writes nothing at all when the prefix does not fit, and still returns
  -1 then.
- No allocation: this module does not own any memory, and the fix does not need any. A fixed
  buffer or the caller's own buffer is enough.
- Do not add a `const` cast to silence something. If a pointer should have been `const`,
  making it `const` is part of the fix, not a workaround.

## Verify

```bash
make check
```

Done means `s00-e03-literal-write: all checks passed` in the plain stage and again in the
ASan stage, with no signal and no report, and `== check: OK`.

A useful experiment afterwards, in a scratch file outside this directory: change
`char *name = "warning";` to `const char *name = "warning";` in a copy of the broken code and
compile it. The error you get (`assignment of read-only location`, or clang's
`read-only variable is not assignable`) is the compiler telling you at build time what the
sanitizer told you at run time. That is why the habit is `const char *` for anything you did
not allocate.

<details><summary>Hint</summary>

`out` is the caller's buffer, and the caller's buffer is writable. The prefix has to be built
there anyway.

</details>
