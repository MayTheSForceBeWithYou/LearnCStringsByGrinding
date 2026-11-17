# s00-e02-count-and-terminate: notes

## Why it is correct

`str_len` walks `s[i]` while the byte is not NUL. The loop ends at the first NUL, and `i` is
then the number of bytes before it, which is the definition of the length. The terminator is
read (that is how the loop knows to stop) and nothing after it is.

`str_end` is the same walk with the pointer as the state. When `*s` is NUL the loop stops with
`s` holding the address of that byte, which is what the function returns. Writing the two in
the two shapes makes the equivalence visible: `str_end(s) - s == str_len(s)` for every string,
and both forms compile to nearly the same instructions.

`buf_is_terminated` is bounded by `cap` and never looks at `buf[cap]`. Because the loop
condition is tested before the body, `cap == 0` reads nothing, which the harness checks with a
zero-byte buffer: any dereference there is a report under ASan.

`buf_set` writes `min(n, cap - 1)` copies and then the terminator at the index the loop left.
The condition `i + 1 < cap` is the whole exercise: it reserves the last byte, so the
terminator always fits. It is written as `i + 1 < cap` rather than `i < cap - 1` because `cap`
is a `size_t` and `cap - 1` with `cap == 0` is `SIZE_MAX`; the `cap == 0` case is returned
early, so either form would work here, but the habit is worth having.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| empty string | `while (s[i] != '\0')` and `while (*s != '\0')` never run the body; 0 and `s` |
| embedded NUL | both loops stop at the *first* NUL; the bytes after it are not part of the string |
| bytes above 0x7F | the test is `!= '\0'`, never `> 0`, so the sign of `char` is irrelevant |
| `cap == 0` in `buf_is_terminated` | `i < cap` is false immediately; `buf` is never read |
| `cap == 0` in `buf_set` | the early `return 0`, before any write |
| `cap == 1` | the loop condition `i + 1 < cap` is false at `i == 0`; only `buf[0] = '\0'` |
| `n == 0` | the loop condition `i < n` is false at `i == 0`; only the terminator |
| `n == SIZE_MAX` | `i < n` and `i + 1 < cap`; `n` is never incremented, so nothing wraps |
| `c == '\0'` | stored like any other byte; the result is a string of length 0 |
| bytes past the terminator | never written: the loop stops at `i`, and only `buf[i]` follows |

## Alternatives

The library call that replaces each of these:

| This function | The library |
|---|---|
| `str_len` | `strlen(s)` (S01) |
| `str_end` | `s + strlen(s)`, or `stpcpy`'s return value when you are copying anyway (S03) |
| `buf_is_terminated` | `memchr(buf, '\0', cap) != NULL` (S02), or `strnlen(buf, cap) < cap` (S01) |
| `buf_set` | `memset(buf, c, n); buf[n] = '\0';` after checking `n < cap` (S02) |

`buf_set` is the only one whose library version needs a decision from the caller first, and
that decision is the subject of S03 e06: truncate, or refuse.

A pointer-difference `str_len` (`return str_end(s) - s;`) is a legitimate third shape, and it
is what drill r01 asks for. It returns `ptrdiff_t` converted to `size_t`, which is fine for
any string that fits in memory.

## The usual mistake

Bounding `buf_set`'s loop by `cap` rather than `cap - 1`, and then writing the terminator at
`buf[i]`, which is `buf[cap]`: one byte past the end, every time the request is as large as
the buffer. The harness catches it three ways at once (a wrong return value, a buffer that is
not terminated, and a fence failure), and ASan reports `WRITE of size 1 ... 0 bytes after
8-byte region`. The same off-by-one, with `strcpy` instead of a loop, is the single most
common memory-safety defect in C.

Second: `str_end` returning `s + str_len(s) + 1`, one past the terminator. That is the
*next write position* for an appender, not the end of the string, and it is a useful pointer,
but it is not what this function promises. Confusing the two is how an appender overwrites
the byte after a buffer.

Third: making `buf_is_terminated` call `strlen` and compare with `cap`. It is forbidden here,
but it is also wrong: `strlen` on a buffer with no terminator is exactly the undefined
behaviour the function exists to rule out. The bounded question needs a bounded loop.

## How libc does it

`strlen` in glibc and in Apple's libc reads a machine word or a vector register at a time:
load 16 to 64 aligned bytes, compare them all against zero in one instruction, and if any byte
matched, find which. The loop therefore runs once per 16 to 64 bytes instead of once per byte,
and it may read bytes past the terminator, which is safe only because it never crosses the
aligned block it has already touched and so cannot leave a mapped page. A portable C program
is not allowed to do this; a libc, compiled for one platform, is.

`memset` is the model for `buf_set`'s inner loop and gets the same treatment: wide stores,
often with a separate path for small sizes, and on x86-64 sometimes `rep stosb`, which modern
chips implement in microcode faster than a hand-written loop. None of that changes the
interface, and none of it decides anything about the terminator: that is the caller's
business, which is the point of this exercise.
