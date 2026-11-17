# s00-r01-walk-reps: notes

## Why it is correct

Each function is one `while (*p != '\0')` walk, and the terminator is the only thing that
stops it. `walk_len` keeps the start so that the length is the pointer difference, which is
the pointer-walk form of the same answer `str_len`'s index gives. `walk_end` returns the
pointer the loop is holding when it stops, which is by definition the address of the
terminator.

`walk_count` returns 0 for `c == '\0'` before walking at all: the terminator is not one of
the characters of the string, so the honest answer is zero rather than one. Without that
early return the loop would stop before ever comparing the terminator and would return 0
anyway, but saying it in one line makes the contract visible.

`walk_last` tests `p[1]` rather than `p[0]`, so it stops with `p` on the last character. That
is safe only because the empty case is returned first: with `*p == '\0'` already handled,
`p[1]` is never read past the terminator. The cast to `unsigned char` before returning is what
makes `"\xFF"` give 255 instead of -1, which matters because -1 is this function's "empty"
answer.

`walk_is_empty` reads exactly one byte. Writing it as `walk_len(s) == 0` would read the whole
string to answer a question about its first byte.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | every loop's condition is false at once; `walk_last`'s early `return -1` |
| a string ending in a byte above 0x7F | `(unsigned char)*p` in `walk_last` |
| `c == '\0'` in `walk_count` | the early `return 0` |
| bytes after the first terminator | no loop looks past it, so `"ab\0cd"` is `"ab"` |
| a one-character string in `walk_last` | `p[1]` is the terminator, so the loop body never runs |
| reading `s[len + 1]` | never happens: every loop tests the byte it is about to use |

## Alternatives

| This function | The library call | Unit |
|---|---|---|
| `walk_len` | `strlen(s)` | S01 |
| `walk_end` | `s + strlen(s)`, or `stpcpy`'s return value while copying | S03 |
| `walk_count` | no single call: `strchr` in a loop, or `memchr` over `strlen` bytes | S08 |
| `walk_last` | `s[strlen(s) - 1]`, with the length-zero case tested first | S01 |
| `walk_is_empty` | `s[0] == '\0'`, or `*s == '\0'` | — |

`walk_is_empty` is the one worth noticing: the library version *is* the one-liner, and
`strlen(s) == 0` is the version that costs a full pass over a string whose length nobody
wanted. S12 e08 is the same mistake with a buffer: `buf[strlen(buf) - 1]` on an empty string
writes `buf[-1]`.

The index form of all five is equally correct and compiles to nearly the same code; writing
them with a pointer here is a drill, not a performance claim. What the pointer form buys is
that `walk_end` needs no arithmetic at all.

## The usual mistake

`walk_last` written as `s[walk_len(s) - 1]` without testing for the empty string first. The
length is `size_t`, so `0 - 1` is `SIZE_MAX`, and the subscript reads somewhere far outside
the object. Under ASan it is a wild read; without it the function returns a byte from another
object. That is S01 e03, and it is the single most common `size_t` defect in C.

Second: returning `*p` rather than `(unsigned char)*p` from `walk_last`. On a machine where
`char` is signed, a final byte of `0xFF` becomes -1, which is this function's code for "the
string is empty", so a caller now cannot distinguish them. Keep data bytes in
`unsigned char` the moment they become numbers.

Third: `walk_count` advancing before comparing (`p++; if (*p == c)`), which skips the first
character and reads one byte past the terminator. The harness's exactly-sized buffers turn
that into a report rather than a near miss.

## How libc does it

None of these loops is what the library actually runs. glibc's `strlen` loads an aligned
machine word (or a 16- to 64-byte vector) and tests all of its bytes for zero at once with a
bit trick or a vector compare, so the loop runs once per word rather than once per byte.
`memchr` and `strchr` do the same for an arbitrary byte, by broadcasting it to every lane and
comparing in parallel.

Two consequences worth carrying. First, a hand-written byte loop is typically five to twenty
times slower than the library call on long strings, so "I will just walk it myself" is a real
cost in a hot path. Second, those implementations may read bytes past the terminator inside
an aligned block they have already touched, which a portable program is not allowed to do —
which is exactly why this drill's harness gives you buffers of their exact size, and why your
version has to be strict where libc's is allowed not to be.
