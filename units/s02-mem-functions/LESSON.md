# S02: The `mem` functions

## Organizer

Underneath every string function in C are five that do not know what a string is. `memcpy`,
`memmove`, `memset`, `memcmp` and `memchr` take a pointer and a **count**, touch exactly that
many bytes, and never look for a terminator. `strcpy` is `memcpy` with the length found by
walking, `strcmp` is `memcmp` with the same, and a bounded `strnlen` is `memchr` looking for a
zero byte. Learn these five and the rest of `<string.h>` is a convenience layer.

What changes in your thinking: the question "how long is it?" is replaced by "how many bytes
am I moving?", and the answer comes from the *program* rather than from the data. That removes
a whole class of defect — nothing can run off the end looking for a terminator that is not
there — and introduces two new ones, which are this unit's `[BUG]` exercises: a count that is
in the wrong argument, and a count that includes bytes which are not part of the value.

You need S01 (`size_t`, lengths against sizes, bounded measurement). S00's byte notation and
`unsigned char` rule are used throughout.

After this unit you can:

- say for each of the five what it reads, writes, returns and requires of the caller, and
  choose between `memcpy` and `memmove` from the two pointers;
- write all five from a blank file, correct for `n == 0`, for every overlap and for bytes
  above 0x7F;
- read a `memcpy-param-overlap` report and say by how many bytes the areas overlapped;
- say why comparing two structs with `memcmp` is wrong while copying them with `memcpy` is
  not, and build generic array operations over `(base, n, size)`.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| `memcpy` | `<string.h>` | ISO C | below | `n` bytes, regions must not overlap; returns `dst` |
| `memmove` | `<string.h>` | ISO C | below | `n` bytes, any overlap; returns `dst` |
| `memset` | `<string.h>` | ISO C | below | `n` copies of one byte; returns `s` |
| `memcmp` | `<string.h>` | ISO C | below | first `n` bytes as `unsigned char`; sign only |
| `memchr` | `<string.h>` | ISO C | below | first byte equal to one value, or NULL |
| `strlen` | `<string.h>` | ISO C | S01 | used here only to measure what a copy will move |

All five are ISO C and need no feature-test macro. All five take their count as a `size_t`,
accept 0 for it, and require valid pointers even then (C17 7.24.1p2).

## Foundations

### A count, not a convention

The `str` functions and the `mem` functions divide the library in two, and the line between
them is where the length comes from:

| | `str*` | `mem*` |
|---|---|---|
| Where the length comes from | the data: a terminator | the caller: a `size_t` |
| Stops early | at the first NUL | never (except `memcmp`/`memchr` on a decision) |
| A NUL byte in the data is | the end | an ordinary byte |
| Can run off the end of an object | yes, if there is no terminator | only if the count is wrong |
| Needs the data to be a string | yes | no |
| Suitable for | text you terminated yourself | fields, records, images, anything with a size |

That table is why this unit comes before copying, comparison and searching. Each of those has
a `str` family whose defects are "the terminator was not where I assumed" and a `mem` family
whose defects are "the count was not what I meant" — and the second kind is easier to reason
about and to test, which is why careful C uses the `mem` functions wherever the size is known.

### Overlap, and what `restrict` promises

`memcpy`'s parameters are declared `restrict`; `memmove`'s are not. That single difference is
the whole difference between the two functions:

```c
void *memcpy(void *restrict dst, const void *restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
```

`restrict` is a promise **by the caller** (C17 6.7.3.1): during the call, the bytes reached
through one pointer are not reached through the other. It is not checked, it generates no code,
and breaking it is undefined behaviour. In exchange the implementation may copy in any order,
in any sized pieces, with whatever instructions the processor has.

`memmove` makes no such demand. C17 7.24.2.2 defines its result exactly: the bytes are copied
"as if" the source had first been copied into a temporary array and then from there to the
destination. No real implementation uses a temporary; they compare the two pointers and choose
a direction:

```
dst below src: copy forwards      dst above src: copy backwards
  src: a b c d e f                  src: a b c d e f
  dst:     ^                        dst:         ^
  in both, every byte is read before the write that would overwrite it
```

Get the direction wrong and the first bytes written destroy source bytes not yet read, so the
output is the first few bytes repeated. The demo shows both directions and `dst == src`:

```
what each call leaves in an 8-byte buffer holding ########

  call                             buffer after         note
  ----                             ------------         ----
  memset(buf, 'x', 3)              xxx#####             three bytes, the rest untouched
  memset(buf, 0x141, 4)            AAAA####             0x141 converted to unsigned char: 'A'
  memset(buf, -1, 2)               \xFF\xFF######       and -1 becomes 0xFF
  memset(buf, 'x', 0)              ########             n == 0 writes nothing
  memmove(buf + 2, buf, 6)         ababcdef             dst above src: copied from the end
  memmove(buf, buf + 2, 6)         cdefghgh             dst below src: copied from the start
  memmove(buf, buf, 8)             abcdefgh             dst == src: nothing changes
```

Why not always call `memmove`? Because `restrict` buys real speed on the short fixed-size
copies that dominate — a struct assignment compiles to `memcpy`, never `memmove` — and because
the name states the caller's intent. And `memcpy` with overlapping areas does usually produce
the right bytes, which is the problem: "usually" is not a promise. See **What the tools say**.

### One byte, n times

`memset(void *s, int c, size_t n)` takes the **pointer, then the byte, then the count**, and
converts `c` to `unsigned char` before writing it (C17 7.24.6.1). Two consequences, both in
the demo above: `memset(p, -1, 2)` writes `0xFF 0xFF`, and `memset(p, 0x141, 4)` writes four
`'A'`s, because 0x141 truncated to eight bits is 0x41.

The parameter order is the one people get wrong, because two of the three parameters are
integers and a swap compiles silently. `memset(p, sizeof *p, 0)` fills **zero** bytes with the
value `sizeof *p`: nothing is written, nothing is reported, and the object keeps every byte it
had. e04 is a program that cleared its records that way.

There is no rule to deduce the order from — `memcpy` is (dst, src, n), `fwrite` is (ptr, size,
nmemb, stream) — so it is the card, or the page, or a compiler that can see the constant.

### Bytes compare as `unsigned char`, and only the sign is fixed

`memcmp` compares the first `n` bytes "each interpreted as `unsigned char`" (C17 7.24.4.1) and
returns a value whose **sign** says which is greater. Two things follow, and the demo shows
both:

```
memcmp compares bytes as unsigned char, and promises only a sign

  comparison                     sign   why
  ----------                     ----   ---
  memcmp("abc", "abc", 3)           0   equal
  memcmp("abc", "abd", 3)          -1   'c' < 'd'
  memcmp("abc", "abd", 2)           0   the difference is past n
  memcmp("\x7F", "\x80", 1)        -1   0x7F < 0x80 as unsigned char
  memcmp("\xFF", "\x01", 1)         1   0xFF > 0x01, not negative
  memcmp(a, b, 0)                   0   no bytes compared
```

**The comparison is unsigned**, on every platform, whatever plain `char` is. A comparison you
write with `char` pointers gets `0x7F`/`0x80` and `0xFF`/`0x01` backwards here, and gets every
ASCII case right — which is why the defect survives testing (S06 e03 is a binary search that
misses keys for it).

**Only the sign is specified.** Not -1 and 1, not the byte difference; glibc has returned both
over the years. Code that tests `memcmp(a, b, n) == 1` is broken and passes its tests, which
is why this course's harnesses compare `sign_of()` on both sides.

One property not in the standard and worth knowing: `memcmp` may read all `n` bytes even after
finding a difference, and a vectorised version does. So **it is not constant-time** and must
not be used on a password, a hash or a MAC.

### A struct has bytes that are not members

C17 6.7.2.1p15 allows unnamed padding between a structure's members and after the last one,
and implementations use it to align each member. For this struct on both course platforms:

```c
typedef struct { char kind; int value; } Point;   /* 8 bytes, not 5 */
```

```
offset:   0    1    2    3    4    5    6    7
          kind |--- padding ---| |---- value ----|
```

The padding is part of the object — copied by assignment and by `memcpy`, written to a file if
you write the struct to a file — and **its values are unspecified**: assigning `p.kind` and
`p.value` says nothing about bytes 1 to 3 (C17 6.2.6.1p6). So two `Point`s with identical
members can differ in three bytes of eight:

```
a struct has bytes that are not members

  sizeof(Point)                = 8  (char + int is 5 bytes of members)
  p, padding zeroed            = p\0\0\0*\0\0\0
  q, padding 0x5A              = pZZZ*\0\0\0
  p.kind == q.kind && p.value == q.value : yes
  memcmp(&p, &q, sizeof p)               : NOT equal
```

So `memcmp(&a, &b, sizeof a) == 0` answers "are these objects byte-for-byte identical", which
is a different question from "do these records hold the same values". The asymmetry to
remember: **copying all the bytes is fine; comparing all the bytes is not.** e05 is a table
lookup that compared them.

### Generic arrays: `(base, n, size)`

C has no generic types, so the library's generic functions take the element size as a
parameter: `qsort(base, nmemb, size, cmp)`, `bsearch(...)`, `calloc(nmemb, size)`. Element `i`
is the `size` bytes at `base + i * size`, and the only operations available on it are the `mem`
functions. Two habits follow: write `sizeof *p` rather than `sizeof(Rec)` at a call site, so
the size cannot drift from the type; and when an element is bigger than your scratch buffer,
move it in pieces, which is what `qsort` does internally and what e07 asks for.

## Function reference

### `memcpy`

```c
#include <string.h>
void *memcpy(void *restrict dst, const void *restrict src, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.2.1). Linux STANDARDS: `C11, POSIX.1-2008`. |
| **Does** | Copies `n` bytes from `src` to `dst`. Looks at nothing: no terminator, no early exit. |
| **Returns** | `dst`. Never an error indication; it cannot fail. |
| **Requires** | `n` bytes readable at `src`, `n` writable at `dst`, and the two regions **do not overlap**. Both pointers valid even when `n` is 0. |
| **Undefined if** | The regions overlap; either region is shorter than `n`; either pointer is null (even with `n == 0`). |
| **Reads / writes** | Reads exactly `n`, writes exactly `n`, in any order the implementation likes. |
| **Terminated afterwards? / Reports truncation?** | Only if a terminator was among the `n` bytes; this function writes none. There is no truncation, only a count. |
| **Why it is shaped this way** | The `restrict` is what makes it fast: with no possible aliasing, the compiler may keep source bytes in registers across stores, vectorise without a run-time check, and inline short fixed-size copies into a few wide loads and stores. A struct assignment compiles to `memcpy`. |
| **The usual mistake** | Using it for an in-place edit, where the two regions are both inside one buffer. It usually produces the right bytes, which is worse than failing. |
| **Use instead** | `memmove` whenever the regions might overlap. The cost is one pointer comparison. |
| **Use it for** | Copies between distinct objects: into a fresh buffer, out of a record, between two arrays. |

```
src: a b c d e f   dst: # # # # # #   memcpy(dst, src, 6)  ->  dst: a b c d e f
```

### `memmove`

```c
#include <string.h>
void *memmove(void *dst, const void *src, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.2.2) |
| **Does** | Copies `n` bytes from `src` to `dst`, correctly for **every** overlap, including `dst == src`. The result is as if the source had been copied to a temporary array first. |
| **Returns** | `dst`. |
| **Requires** | `n` bytes readable at `src` and writable at `dst`. Nothing about overlap. |
| **Undefined if** | Either region is shorter than `n`; either pointer is null. |
| **Reads / writes** | Reads exactly `n`, writes exactly `n`. |
| **Terminated afterwards? / Reports truncation?** | As `memcpy`. |
| **Why it is shaped this way** | It is the function for editing a buffer in place — opening a gap, closing one, shifting a tail — which is most of what text and array code does. The "as if a temporary" wording describes the *result* so that the implementation is free to choose a direction instead. |
| **The usual mistake** | Assuming it is much slower than `memcpy` and avoiding it. On both course platforms it is the same vectorised code with a comparison in front. |
| **Use instead** | Nothing. If the regions are known to be disjoint, `memcpy` says so to the reader and to the compiler; otherwise this is the function. |
| **Use it for** | Any move inside one buffer, and any copy where you cannot prove the regions are distinct. |

```
buf:  a b c d e f g h       memmove(buf + 2, buf, 6)  ->  a b a b c d e f   (from the end)
buf:  a b c d e f g h       memmove(buf, buf + 2, 6)  ->  c d e f g h g h   (from the start)
```

### `memset`

```c
#include <string.h>
void *memset(void *s, int c, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.6.1) |
| **Does** | Writes `n` copies of `(unsigned char)c` to the bytes at `s`. Reads nothing. |
| **Returns** | `s`. |
| **Requires** | `n` bytes writable at `s`; a valid pointer even when `n` is 0. |
| **Undefined if** | Fewer than `n` bytes are writable; `s` is null. |
| **Reads / writes** | Writes exactly `n` bytes, reads none. |
| **Terminated afterwards?** | With `c == 0` and `n >= 1`, the region is all zero bytes, so it is the empty string — and `n` zero bytes, not one. |
| **Reports truncation?** | none. |
| **Why it is shaped this way** | The `int` value parameter is a fossil of pre-prototype C, where a `char` argument was promoted to `int` anyway; the standard keeps the signature and specifies the conversion instead. |
| **The usual mistake** | The argument order: `memset(p, sizeof *p, 0)` fills zero bytes with the value `sizeof *p`, silently. Second: expecting it to survive optimisation when clearing a secret — a compiler may remove a `memset` whose result is never read. |
| **Use instead** | `= {0}` for a struct (typed, and no argument order to get wrong); `calloc` when the memory is being allocated anyway; `memset_explicit` (C23) for secrets. |
| **Use it for** | Clearing a record or an array before reuse, filling a buffer with a pad byte, and the `'#'` fill this course's harnesses use to make unwritten bytes visible. |

```
buf: # # # # # # # #   memset(buf, 'x', 3) -> xxx#####   memset(buf, 0x141, 4) -> AAAA####
                       memset(buf, -1, 2)  -> \xFF\xFF######
```

### `memcmp`

```c
#include <string.h>
int memcmp(const void *s1, const void *s2, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.4.1) |
| **Does** | Compares the first `n` bytes at `s1` and `s2`, each interpreted as `unsigned char`. |
| **Returns** | An `int` whose **sign** says which is greater: negative if the first differing byte is smaller in `s1`, positive if larger, 0 if the `n` bytes are equal. The magnitude is unspecified. `n == 0` returns 0. |
| **Requires** | `n` bytes readable at each; valid pointers even when `n` is 0. |
| **Undefined if** | Either region is shorter than `n`; either pointer is null. |
| **Reads / writes** | Reads at most `n` bytes of each — and is allowed to read all of them even after finding a difference. Writes nothing. |
| **Terminated afterwards? / Reports truncation?** | none. |
| **Why it is shaped this way** | Specifying only the sign lets an implementation compare whole words and work out the order from a bit scan, without computing which byte differed. Specifying `unsigned char` makes the order the same on every platform. |
| **The usual mistake** | Comparing the result with 1 or -1. Second: using it on two structs, where it compares the padding too (e05). Third: using it on secrets, where its early exit and its full-read are both timing leaks. |
| **Use instead** | Member-by-member comparison for structs; a constant-time loop for secrets; `strcmp`/`strncmp` for strings, which are this function with the length discovered. |
| **Use it for** | Fixed-width fields, byte buffers, image rows, "did this block change" — anything whose bytes *are* its value. |

```
"abc" vs "abd", n = 3  -> negative   "abc" vs "abd", n = 2  -> 0 (difference past n)
"\x7F" vs "\x80", n = 1 -> negative   "\xFF" vs "\x01", n = 1 -> positive (255 > 1)
```

### `memchr`

```c
#include <string.h>
void *memchr(const void *s, int c, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.5.1) |
| **Does** | Finds the first byte equal to `(unsigned char)c` among the first `n` bytes at `s`. |
| **Returns** | A pointer to that byte, or **NULL** if it is not among the `n`. `n == 0` returns NULL. |
| **Requires** | `n` bytes readable at `s`; a valid pointer even when `n` is 0. |
| **Undefined if** | Fewer than `n` bytes are readable; `s` is null. |
| **Reads / writes** | Reads at most `n` bytes, and may stop at the byte it finds. Writes nothing. |
| **Terminated afterwards?** | none. |
| **Reports truncation?** | NULL is "not found", which is the only report it makes. |
| **Why it is shaped this way** | It is the bounded search the `str` family lacks, and it is the primitive under a bounded length: `strnlen(s, n)` is `(memchr(s, '\0', n) ?: s + n) - s`, which the Linux `strnlen(3)` page gives as the definition. |
| **The usual mistake** | Forgetting that it takes a `const void *` and returns a `void *`, so the `const` is silently dropped; and assuming it stops at a NUL byte, which it does not — a NUL is found like any other byte and searched past like any other byte. |
| **Use instead** | `strchr` when the data is a string and you want to stop at the terminator anyway (S08). |
| **Use it for** | Finding a delimiter in a buffer of known size, finding the terminator in a fixed-width field, splitting a `read()` buffer into lines. |

```
s: a b \0 c b d    memchr(s,'b',6) -> s+1 (the first)   memchr(s,'\0',6) -> s+2 (an ordinary byte)
                   memchr(s,'d',6) -> s+5 (past the NUL)  memchr(s,'b',1) -> NULL (n first)
                   memchr(s,'z',6) -> NULL
```

## Worked example: deleting four characters from a buffer

A 30-character string in a buffer, and the first four characters have to go. The bytes after
them move down, and the terminator moves with them.

```c
size_t move = len - at - n + 1;              /* the tail, and the terminator */
memmove(buf + at, buf + at + n, move);
```

With `len` 30, `at` 0 and `n` 4: `move` is 27 — twenty-six characters plus the terminator. The
destination is `buf + 0`, the source is `buf + 4`, and the two regions are
`[buf, buf+27)` and `[buf+4, buf+31)`, which overlap by 23 bytes. `memmove` is defined for
that; `memcpy` is not.

Three details. The `+ 1` is the terminator: forget it and the old terminator stays four bytes
too far along, so the string is four characters longer with stale bytes at the end. The
arithmetic `len - at - n` is safe only because `at + n <= len` was established first — two
unsigned subtractions in a row. And the destination is *below* the source, the safe direction
for a forward byte loop, which is why the `memcpy` version of this code survives in so many
programs.

**The plausible wrong reading.** "The destination is below the source, so `memcpy` is fine
here — overlap only matters when you shift text up." It is fine *on this implementation, at
this size, today*. `restrict` is a promise and the program has broken it; glibc's x86-64
`memcpy` copies large blocks from both ends inwards, and this call survives only because 27
bytes takes a path that happens to be safe for this direction. The `memcpy(3)` page's CAVEATS
records the release where that stopped being true for many programs at once. e03 is this code:
`make run` prints the right answer and `make asan` reports the defect.

## Worked example: two records that are equal and do not compare equal

```c
typedef struct { char kind; int value; } Point;

Point a = point_make('p', 42);       /* in storage that was zeroed      */
Point b = point_make('p', 42);       /* in storage that held 0x5A bytes */
memcmp(&a, &b, sizeof a) == 0        /* false */
```

Both have `kind == 'p'` and `value == 42`, and the member comparison agrees. The byte
comparison does not, because the objects are eight bytes of which three are padding:

```
a:   'p'  00 00 00   2A 00 00 00
b:   'p'  5A 5A 5A   2A 00 00 00
          ^^^^^^^^ these three bytes are not part of either record's value
```

Assigning the members leaves bytes 1 to 3 as they were, with unspecified values (C17
6.2.6.1p6), so `memcmp` is answering a question about the objects rather than the records — and
answering it correctly. The repair is to compare members. Note which call in the same program
is *not* wrong: copying the struct with `memcpy` carries the padding along, which is harmless
and is exactly what `*dst = *src;` does.

**The plausible wrong reading.** "Zero the storage before filling it and `memcmp` works, so
this is a convention the program can adopt." It is a whole-program obligation: every `Point`
anywhere — one on a stack slot filled member by member, one `memcpy`ed out of a file, one in a
`realloc`ed array — must have been zeroed first, and C cannot force values through a
constructor. One place forgets, and a table lookup starts missing keys that are certainly
there. Comparing members is local, cheap, and cannot be undone by code you have not read.
e05 is the lookup.

## What the tools say

**The compiler, when it can see the constants.** `lesson-demo/warn.c` holds three mistakes of
this unit. From `lesson-demo/capture-warn.txt` (gcc at `-O2 -Wall -Wextra -Wpedantic`):

```
warn.c: In function ‘clear_point’:
warn.c:15:5: warning: ‘memset’ used with constant zero length parameter; this could be due to transposed parameters [-Wmemset-transposed-args]
   15 |     memset(p, sizeof *p, 0);
      |     ^~~~~~
warn.c: In function ‘clear_through’:
warn.c:27:25: warning: argument to ‘sizeof’ in ‘memset’ call is the same expression as the destination; did you mean to provide an explicit length? [-Wsizeof-pointer-memaccess]
   27 |     memset(p, 0, sizeof p);
      |                         ^
```

and from `capture-warn-clang.txt`, the same file through clang:

```
warn.c:15:26: warning: 'size' argument to memset is '0'; did you mean to transpose the last two arguments? [-Wmemset-transposed-args]
   15 |     memset(p, sizeof *p, 0);
      |                          ^
warn.c:27:25: warning: 'memset' call operates on objects of type 'char' while the size is based on a different type 'char *' [-Wsizeof-pointer-memaccess]
   27 |     memset(p, 0, sizeof p);
      |            ~            ^
      (two "did you mean" notes omitted)
```

Two of the three are caught by both compilers, in their own words: the transposed `memset`
arguments (because the count is a literal zero they can see) and `sizeof` on a pointer used as
a byte count. Both diagnostics need a **constant**; move the transposed call behind a two-line
helper whose arguments are parameters and it vanishes, which is what e04 does and what every
real code base does by accident. Neither compiler says anything about the third function, a
`memcpy` whose two arguments are the same buffer: overlap is a run-time property of two
addresses, and that is what the sanitizer is for.

**AddressSanitizer, for overlap.** `make asan ARGS=overlap` in `lesson-demo` calls `memcpy`
with overlapping regions; from `capture-overlap.txt` (paths and build ids shortened):

```
==1807955==ERROR: AddressSanitizer: memcpy-param-overlap: memory ranges [0x71f8d82d0080,0x71f8d82d0097) and [0x71f8d82d0084, 0x71f8d82d009b) overlap
    #0 0x75f8db129547 in memcpy (/usr/lib/libasan.so.8+0x129547)
    #1 0x5a63d47445be in copy .../lesson-demo/main.c:30
```

A report with no out-of-bounds byte in it: no "is located N bytes after", no allocation stack,
no object name — just the error's name and **two half-open ranges**, the destination first,
because that is the order of `memcpy`'s parameters. Everything is in that line: each range's
length is `end - start`, the distance between them is the difference of the starts, and the
overlap is the length minus the distance (23 bytes here). Frame `#0` is ASan's interceptor,
`#1` is the call. e03 asks for that arithmetic.

**Nothing at all.** The unit's other two defects produce no report from any tool, in any build.
A transposed `memset` behind a helper writes zero bytes: nothing out of bounds, nothing
undefined, and the records keep their old contents (e04). A `memcmp` over a struct reads
padding, which is allowed, and compares values the standard calls unspecified (e05). Both are
caught by checks, and only by checks.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| `str*` vs. `mem*` | The length comes from the data, or from the caller. |
| `memcpy` vs. `memmove` | `memcpy` requires disjoint regions and says so with `restrict`; `memmove` is defined for every overlap. The code they run is nearly the same. |
| "works" vs. "defined" | A `memcpy` with overlapping regions usually produces the right bytes, and is still undefined behaviour. |
| `memcmp`'s value vs. its sign | Only the sign is specified. Comparing with 1 or -1 is a bug that passes its tests. |
| comparing a struct vs. copying one | Copying all the bytes is fine; comparing them compares the padding, which is not part of the value. |

## `man` pointers

- `man 3 memcpy`: SYNOPSIS for the `restrict` qualifiers, DESCRIPTION for "the memory areas
  must not overlap", and **CAVEATS** for the glibc 2.13 story — the best short account anywhere
  of what undefined-behaviour-that-works is worth.
- `man 3 memmove`: one paragraph, which is the whole point.
- `man 3 memset`, `man 3 memcmp`, `man 3 memchr`: DESCRIPTION in each, for the `unsigned char`
  conversion, the `unsigned char` comparison and the NULL return. `man 3 string` indexes the
  whole family next to their `str` relatives.
- `man 7 string_copying` (Linux only), **Terms**: *length-bounded character sequence* names the
  data these five work on.

## Check yourself

1. `memset(buf, 12, 0)` and `memset(buf, 0, 12)`: what does each write?
2. `memcmp("\xFF", "\x01", 1)`: what is its sign, and what would a hand-written comparison
   with `char` pointers give on this platform?
3. Two `Point`s (one `char`, one `int`) hold the same values. Give a reason why
   `memcmp(&a, &b, sizeof a)` might be non-zero, and name the byte offsets involved.
4. A 40-byte buffer holds a 20-character string at offset 0. Write the call that deletes the
   first three characters, and say how many bytes it moves.
5. `memchr(s, 'x', 0)` and `memchr(s, '\0', n)` where `s` starts with a NUL: what does each
   return?
6. Why is `memcmp` unsuitable for comparing a password hash?
7. You have an array of records of 100 bytes each and a 32-byte scratch buffer. How do you
   exchange two of them?

<details><summary>Answers</summary>

1. The first writes **nothing**: zero bytes of the value 12. The second writes twelve zero
   bytes. That pair is e04's defect.
2. Positive: as `unsigned char`, 255 > 1. With `char` pointers on this platform the result is
   negative, because `0xFF` is -1 and `0x01` is 1.
3. The three padding bytes at offsets 1, 2 and 3: their values are unspecified, and assigning
   the members says nothing about them.
4. `memmove(buf, buf + 3, 20 - 0 - 3 + 1)` — 18 bytes: seventeen characters and the
   terminator.
5. `memchr(s, 'x', 0)` is NULL: no byte is read. `memchr(s, '\0', n)` returns `s`: a NUL is
   found like any other byte.
6. Timing. It may return at the first difference, and a vectorised version may read all `n`
   bytes regardless; neither is specified, so the time can depend on the data. Secrets need a
   loop that always does the same work.
7. In pieces: 32 bytes through the scratch buffer, then 32, then 32, then the last 4. That is
   e07's `mem_swap`, and what `qsort` does for large elements.

</details>

## Takeaways

- The `mem` functions take a count and look at nothing else; the length comes from the program,
  not from the data.
- `restrict` on `memcpy` is a promise by the caller, unchecked and undefined to break — and a
  broken one usually still produces the right bytes, which is a property of this afternoon
  rather than of the program. Use `memmove` for any move inside one buffer.
- `memset` takes the byte before the count, and converts it to `unsigned char`: `-1` fills with
  0xFF.
- `memcmp` compares as `unsigned char` and specifies only the sign; it is not constant-time.
- A struct's padding is part of its bytes and not part of its value, so compare members and
  copy bytes.
- The generic-array interface is `(base, n, size)`, and an element larger than your scratch
  buffer is moved in pieces.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-my-memcpy-memmove/` | `[WRITE]` | M | both copies, with an overlap sweep from −32 to +32 |
| e02 | `e02-my-memset-memcmp-memchr/` | `[WRITE]` | S | the other three, with the `unsigned char` rules |
| e03 | `e03-overlap-report/` | `[REPORT]` | | a `memcpy-param-overlap` report, and the arithmetic on its two ranges |
| e04 | `e04-memset-args/` | `[BUG]` | | `memset(p, size, 0)`: nothing is cleared, nothing is reported |
| e05 | `e05-struct-compare/` | `[BUG]` | | `memcmp` on two structs whose padding differs |
| e06 | `e06-mem-predict/` | `[PREDICT]` | | twenty-one outcomes: signs, offsets, and buffers after overlapping moves |
| e07 | `e07-swap-reverse-rotate/` | `[WRITE]` | S | generic swap, reverse and rotate over `(base, n, size)` |
| e08 | `e08-mem-lookup/` | `[LOOKUP]` | | nine facts from the manual pages, with quotes |

## Drills

| # | Folder | Skill | Target |
|---|---|---|---|
| r01 | `drills/r01-gap-reps/` | open, close, delete, insert: `memmove` with the counts right | 12 min |
| r02 | `drills/r02-memset-memcmp-reps/` | clear an array, compare ranges, common prefix, all-one-byte | 10 min |
| r03 | `drills/r03-mem-from-scratch/` | all five `mem` functions from a blank file | 15 min |
