# s03-e05-unterminated-name `[BUG]`

## Goal

A small module stores club members with a 12-byte name field. It works for every name you
would think to try and breaks for long ones. Find the one defect, fix it at its source, and
explain the mechanism. Afterwards you can read an over-read report and walk it back from the
function that crashed to the copy that caused it, which is usually somewhere else entirely.

## Background

This is the most common string defect in C that involves a library function doing exactly
what its documentation says. Nothing here is exotic: a fixed-size field, a bounded copy into
it, and later code that treats the field as a string.

The important habit this exercise builds is separating **where the tool points** from **where
the defect is**. A tool reports the instruction that touched memory it should not have. For
an over-read, that instruction is in an innocent reader (`strlen`, `printf`, `strcmp`) that
was handed something that is not a string. The defect is wherever the terminator went
missing. Fixing the reader (adding a length limit to the one call that crashed) leaves every
other reader of the same field broken.

Read **What the tools say** in `LESSON.md` before you start; it explains each line of a
report like the one below.

## Symptom

`make check` fails in the plain build without crashing:

```
== plain+dsalloc
FAIL main.c:18: the name field holds a terminator
FAIL main.c:19: the stored name has the expected length
FAIL main.c:20: the stored name
       got:  Maximilienne\0
       want: Maximilienn\0
```

The first four members pass. Members 5 and 6 fail. (What `got` shows after the twelfth
character depends on what happens to lie in memory after the record, so your output may
differ there. That in itself is information.)

Because the plain stage fails, `make check` stops before the sanitizer stage. Run that stage
by itself:

```bash
make asan
```

It stops at the first bad access. From `capture.txt`, which was produced from exactly this
source (addresses will differ on your run):

```
==1494029==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6e3a65ee00a0 at pc 0x721a67c8841c bp 0x7ffc3c3c0610 sp 0x7ffc3c3bfdc8
READ of size 13 at 0x6e3a65ee00a0 thread T0
    #0 0x721a67c8841b  (/usr/lib/libasan.so.8+0x8841b)
    #1 0x6118d89c6aae in one .../e05-unterminated-name/main.c:19
    #2 0x6118d89c6d78 in main .../e05-unterminated-name/main.c:35

0x6e3a65ee00a0 is located 0 bytes after 16-byte region [0x6e3a65ee0090,0x6e3a65ee00a0)
allocated by thread T0 here:
    #0 0x721a67d2c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1)
    #1 0x6118d89c9b57 in ds_malloc_at .../lib/dsalloc.c:216
    #2 0x6118d89c6e7d in member_new .../e05-unterminated-name/roster.c:16
```

Frame `#0` is inside the sanitizer's own library: it is ASan's replacement for the libc
function that `main.c:19` called. Frame `#1` is the first line of course code. Neither is
where the defect is.

## What the code is supposed to do

`roster.h` is the specification, and it is correct:

- `member_new(id, name)` returns a heap-allocated `Member`, or NULL if allocation fails. It
  copies `name` into the 12-byte `name` field. **The field always holds a string**: a
  terminator somewhere in its 12 bytes. A name longer than 11 characters is cut to its first
  11 characters.
- `member_name_was_cut(m, original)` returns 1 if the stored name is shorter than `original`.
- `member_label(m, out, cap)` writes `"#<id> <name>"` and returns the length it needed.

| `name` passed in | Length | Stored `name` field (12 bytes) | Cut? |
|---|---|---|---|
| `"Ada"` | 3 | `Ada\0` then 8 more bytes | no |
| `""` | 0 | `\0` then 11 more bytes | no |
| `"Bartholomew"` | 11 | `Bartholomew\0` | no: the longest name that fits |
| `"Maximilienne"` | 12 | `Maximilienn\0` | yes |
| `"Wolfeschlegelstein"` | 18 | `Wolfeschleg\0` | yes |

## What to hand in

1. **The fix**, in `roster.c`. There is exactly one defect, in one function, and nothing else
   in the exercise is wrong. Do not change `main.c` or `roster.h`. Do not change
   `member_name_was_cut` or `member_label`: they are the innocent readers.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted.
   - *The mechanism*: which bytes the call writes for `"Maximilienne"`, which byte is
     missing, and which later call reads past the field.
   - *How the tool showed it*: what each part of the ASan report told you (the error kind,
     `READ`, the size 13, frames `#0` and `#1`, the "is located" line, the 16-byte region and
     where that number comes from), and why the plain build does not crash.
   - *The rule you take away*: one sentence.

## Constraints

- The stored name of a member that fits must be unchanged by your fix: `"Bartholomew"` (11
  characters) is stored whole.
- No allocation beyond the one `malloc` already there.
- Fix the copy, not the readers.

## Verify

```bash
make check
```

Done means `s03-e05-unterminated-name: all checks passed` in the plain stage and again in the
ASan stage, with no report, and `== check: OK`.

<details><summary>Hint</summary>

In the report, the region is 16 bytes and the read started 4 bytes into it. Which field is
that, how big is it, and what did the copy put in its last byte?

</details>
