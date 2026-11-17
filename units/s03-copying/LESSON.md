# S03: Copying

## Organizer

Copying a string into a buffer looks like the simplest operation in C, and it is where most
buffer overflows come from. The library offers five ways to do it, each of which answers
"what if the source does not fit?" differently, and two of which answer it badly. This unit
replaces "use the one with an `n` in its name, it is safer" with three questions you ask of
every copy: **how many bytes does it write, is the result a string, and how does the caller
learn that it did not fit?**

You need S02 (`memcpy`, and what `restrict` promises) and S01 (`strlen`, `strnlen`, and the
difference between a length and a size). Memory course M07 covered sizing a *heap* string for
its contents; here the buffer already exists and the contents must be made to fit it.

After this unit you can:

- say, for `strcpy`, `stpcpy`, `strncpy`, `stpncpy` and `snprintf("%s")`, exactly which bytes
  of the destination each call writes and what it returns;
- say whether a buffer holds a string after a given call, without running it;
- write `strcpy`, `stpcpy`, `strncpy`, `stpncpy` and `strlcpy` from a blank file;
- detect truncation correctly with each function that can report it;
- choose between truncating and refusing, and say which is a defect for which kind of data;
- read an over-read report back to the copy that dropped the terminator, and name one
  overflow that AddressSanitizer cannot report.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| `strcpy` | `<string.h>` | ISO C | below | copy the string; no bound; returns `dst` |
| `stpcpy` | `<string.h>` | POSIX.1-2008 | below | the same copy; returns the end |
| `strncpy` | `<string.h>` | ISO C | below | write exactly `n` bytes: characters, then NUL padding; may not terminate |
| `stpncpy` | `<string.h>` | POSIX.1-2008 | below | the same `n` bytes; returns the first NUL, or `dst + n` |
| `snprintf` with `"%s"` | `<stdio.h>` | ISO C | S11 | bounded copy that always terminates; returns the length it needed |
| `strlcpy` | `<string.h>` | BSD; **not in the portable set** | below | bounded copy that always terminates; returns `strlen(src)` |
| `memccpy` | `<string.h>` | XSI, C23; **not in the portable set** | below | copy bytes up to and including a stop byte, at most `n` |

"Not in the portable set" means the function is not declared under the feature-test macros
this course compiles with, so you cannot call it here. You write `strlcpy` yourself in e03.

## Foundations

### Three questions about any copy

A destination buffer has a **capacity**: its total size in bytes. A string of length `len`
occupies `len + 1` of them, so:

```
the string fits   <=>   len + 1 <= cap   <=>   len < cap
```

Write that comparison as `len < cap` or `len >= cap` every time. `len <= cap` is the
off-by-one that forgets the terminator; `len + 1 > cap` is correct until `len` is `SIZE_MAX`.
In this course a parameter named `cap` or `size` always *includes* the terminator (you pass
`sizeof buf`), and one named `n` or `len` is a count of characters.

Every copy function then answers three questions, and the answers are the function:

| Function | Bytes written to `dst` | Is `dst` a string afterwards? | How the caller learns it did not fit |
|---|---|---|---|
| `strcpy(dst, src)` | `strlen(src) + 1` | yes | it does not: the bytes are written past the buffer |
| `stpcpy(dst, src)` | `strlen(src) + 1` | yes | it does not |
| `strncpy(dst, src, n)` | exactly `n`, always | only if `strlen(src) < n` | it does not: the return value is `dst` either way |
| `stpncpy(dst, src, n)` | exactly `n`, always | only if `strlen(src) < n` | return value `== dst + n` means "no NUL was written" |
| `snprintf(dst, cap, "%s", src)` | `min(strlen(src), cap - 1) + 1` | yes, if `cap > 0` | return value `>= cap` |
| `strlcpy(dst, src, cap)` | `min(strlen(src), cap - 1) + 1` | yes, if `cap > 0` | return value `>= cap` |

Read the middle column twice. Two of the six can leave you holding something that is not a
string, and they are the two whose names promise a bound.

### What "does not fit" can mean

When the source is longer than the room, a function can do one of four things:

| Policy | The buffer afterwards | Right for | Wrong for |
|---|---|---|---|
| **overflow** (`strcpy`) | the string, and whatever followed the buffer is overwritten | nothing | everything |
| **fill without terminating** (`strncpy`) | `n` characters, not a string | fixed-width fields read with `strnlen` or `%.*s` | anything later passed to `strlen`, `%s`, `strcmp` |
| **truncate and report** (`snprintf`, `strlcpy`) | a shorter string | text for people: log lines, labels, titles | identifiers: a cut path, key or user name is a *different* path, key or user |
| **refuse** (you write it) | unchanged, or the empty string | identifiers; anything compared, opened, or granted access | text where "most of it" is useful |

The library gives you the first three. The fourth is three lines, measure, compare, copy,
and you write it in e06 and in drill r01. Choosing between "truncate" and "refuse" is a
decision about the *data*, and it belongs at the call site where the data is known.

### The return value decides whether calls compose

`strcpy` returns `dst`: the one pointer the caller certainly already has. To append after it
you must find the end again, by walking everything just written. `stpcpy` returns the end:

```c
char *p = stpcpy(buf, dir);      /* p points at the terminator: the next write position */
p = stpcpy(p, "/");
p = stpcpy(p, name);             /* total work: the bytes copied, once */
```

A function that returns where it stopped can be chained; one that returns where it started
cannot. The same idea, as a length, is what `snprintf` and `strlcpy` return. S04 is built on
this, and drill r03 trains it.

### Overlap

All of these functions are declared with `restrict` on both pointers (C17 7.24.2.3: "If
copying takes place between objects that overlap, the behavior is undefined"). `restrict` is
the caller's promise, not something the function checks. To move text within one buffer
(deleting a prefix, say) use `memmove`, which is defined for overlap (S02):
`memmove(s, s + 3, strlen(s + 3) + 1)`.

### What ASan cannot see

AddressSanitizer surrounds each *object* (a heap block, a local variable, a global) with
poisoned redzones and reports any access that lands in one. A struct is one object:

```c
struct account { char user[16]; char role[8]; };   /* 24 bytes, one object */
```

A `strcpy` of a 20-character name into `user` writes 21 bytes: 16 into `user`, 5 into `role`.
All 21 are inside the 24-byte object. No redzone is touched and ASan reports nothing. The
overflow is real, the neighbouring field is corrupted, and only a check on the neighbour's
value shows it. A clean sanitizer run means "no access left its object", not "no field
overflowed". e07 is this defect.

## Function reference

The diagrams are the rows of `lesson-demo/capture.txt` (run `make` there to print them all):
an 8-byte buffer, filled with `#` before each call so that unwritten bytes show.

### `strcpy`

```c
#include <string.h>
char *strcpy(char *restrict dst, const char *restrict src);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.2.3) |
| **Does** | Copies the characters of `src`, then its terminator, to `dst`. |
| **Returns** | `dst`. Never an error indication. |
| **Requires** | `src` is a string. `dst` has room for `strlen(src) + 1` bytes. No overlap. |
| **Undefined if** | `dst` is too small; `src` has no terminator; the objects overlap. |
| **Reads / writes** | Reads `strlen(src) + 1` bytes; writes the same number. One pass. |
| **Terminated afterwards?** | Yes. |
| **Reports truncation?** | It never truncates. It overflows. |
| **Why it is shaped this way** | It dates from when buffers were sized by convention and input was trusted. It takes no size because the idea of checking one came later. |
| **The usual mistake** | Copying data whose length the program does not control (input, a file name, an environment variable) into a fixed buffer. |
| **Use instead** | A bounded copy with a policy: `snprintf` and check, or measure-then-`memcpy`. |
| **Use it for** | Copying into a buffer you just sized from the same string (`malloc(strlen(s) + 1)`), or a literal into a buffer visibly large enough. Even then `memcpy` with the known length is clearer. |

```
index:                    0   1   2   3   4   5   6   7
strcpy(buf, "hello")      h   e   l   l   o   \0  #   #    returns buf
```

### `stpcpy`

```c
#include <string.h>
char *stpcpy(char *restrict dst, const char *restrict src);
```

| | |
|---|---|
| **Standard** | POSIX.1-2008. Not in ISO C17. Needs `_POSIX_C_SOURCE 200809L`, which the course build defines. |
| **Does** | The same copy as `strcpy`. |
| **Returns** | A pointer to the terminator it wrote: `dst + strlen(src)`. |
| **Requires / Undefined if** | As `strcpy`. |
| **Reads / writes** | As `strcpy`. |
| **Terminated afterwards?** | Yes. |
| **Reports truncation?** | No: it is unbounded, like `strcpy`. |
| **Why it is shaped this way** | The copy loop ends holding a pointer to the end. Returning it costs nothing and saves the caller a `strlen`. |
| **The usual mistake** | Assuming it is safer than `strcpy`. It has the same overflow; only the return value differs. |
| **Use instead** | Nothing, when the total size has been checked beforehand. |
| **Use it for** | Building a string from parts into a buffer already sized for all of them. |

```
stpcpy(buf, "hello")      h   e   l   l   o   \0  #   #    returns buf + 5
```

### `strncpy`

```c
#include <string.h>
char *strncpy(char *restrict dst, const char *restrict src, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.2.4) |
| **Does** | Writes exactly `n` bytes to `dst`: the characters of `src` up to its terminator, then NUL bytes until `n` bytes have been written. If `src` has `n` or more characters, copies the first `n` and writes **no terminator**. |
| **Returns** | `dst`. Never an error indication, never a length. |
| **Requires** | `dst` has room for `n` bytes. `src` is a string, or has at least `n` readable bytes. No overlap. |
| **Undefined if** | `dst` is smaller than `n`; `src` is neither terminated nor `n` bytes long; the objects overlap. |
| **Reads / writes** | Reads at most `min(strlen(src) + 1, n)` bytes. Writes exactly `n`, always: a 5-character name into a 4096-byte buffer costs 4096 byte writes. |
| **Terminated afterwards?** | Only if `strlen(src) < n`. |
| **Reports truncation?** | No. |
| **Why it is shaped this way** | It was written to fill fixed-width, NUL-padded fields, such as the 14-byte file names of early Unix directory entries, where a full field has no terminator by design. The padding makes equal names byte-for-byte equal fields. It is not a "safe `strcpy`". |
| **The usual mistake** | `strncpy(dst, src, sizeof dst)` followed by treating `dst` as a string. |
| **Use instead, for strings** | `snprintf(dst, sizeof dst, "%s", src)` and check the return value, or a `strlcpy`-style copy (e03). |
| **Use it for** | Fixed-width fields that are read back with `strnlen` or `%.*s` (drill r02). |

```
strncpy(buf, "hello", 8)  h   e   l   l   o   \0  \0  \0   terminated: yes (5 < 8, padded)
strncpy(buf, "hello", 5)  h   e   l   l   o   #   #   #    terminated: NO (5 == n)
strncpy(buf, "hello", 3)  h   e   l   #   #   #   #   #    terminated: NO (5 > n)
```

### `stpncpy`

```c
#include <string.h>
char *stpncpy(char *restrict dst, const char *restrict src, size_t n);
```

| | |
|---|---|
| **Standard** | POSIX.1-2008. Not in ISO C17. |
| **Does** | Writes the same `n` bytes as `strncpy`. |
| **Returns** | A pointer to the first NUL it wrote (`dst + strlen(src)`) when `src` is shorter than `n`; otherwise `dst + n`. |
| **Requires / Undefined if** | As `strncpy`. |
| **Reads / writes** | As `strncpy`. |
| **Terminated afterwards?** | Only if `strlen(src) < n`. |
| **Reports truncation?** | Partly: `ret == dst + n` means no NUL was written. It cannot tell a source of exactly `n` characters from a longer one. |
| **Why it is shaped this way** | It is to `strncpy` what `stpcpy` is to `strcpy`: the same work, a return value that carries information. |
| **The usual mistake** | Dereferencing the return value as "the terminator": when it equals `dst + n` it points one past the field. |
| **Use instead, for strings** | As `strncpy`. |
| **Use it for** | Filling a field and learning its length, or that it is full, in one call. |

```
stpncpy(buf, "hey", 8)       h   e   y   \0  \0  \0  \0  \0   returns buf + 3
stpncpy(buf, "abcdefgh", 8)  a   b   c   d   e   f   g   h    returns buf + 8 (== buf + n)
```

### `strlcpy`

```c
size_t strlcpy(char *restrict dst, const char *restrict src, size_t size);   /* BSD */
```

| | |
|---|---|
| **Standard** | None in C17 or POSIX.1-2008. OpenBSD (1998), the other BSDs, macOS; glibc since 2.38; POSIX.1-2024. **Not in the portable set.** |
| **Does** | If `size > 0`: copies at most `size - 1` characters and always writes a terminator. No padding. If `size == 0`: writes nothing. |
| **Returns** | `strlen(src)`: the length of the string it tried to create. |
| **Requires** | `src` is a string. `dst` has `size` bytes. No overlap. |
| **Undefined if** | `src` has no terminator (it is read to its end whatever `size` is). |
| **Reads / writes** | Reads all of `src`, even when little of it fits. Writes at most `size` bytes. |
| **Terminated afterwards?** | Yes, if `size > 0`. |
| **Reports truncation?** | Yes: `ret >= size`. Then `ret + 1` is the size that would have been enough. |
| **Why it is shaped this way** | It was designed to replace the `strncpy`-then-terminate idiom with one call that takes `sizeof dst` and cannot produce a non-string. |
| **The usual mistake** | Ignoring the return value, which turns "truncate and report" into "truncate silently". |
| **Use instead, here** | `snprintf(dst, size, "%s", src)`: the same bytes and the same number, as an `int`. |
| **Use it for** | Bounded copies of text where truncation is acceptable and is checked. |

### `memccpy`

```c
void *memccpy(void *restrict dst, const void *restrict src, int c, size_t n);   /* XSI, C23 */
```

| | |
|---|---|
| **Standard** | POSIX (XSI option) and ISO C23. Not in C17. **Not in the portable set.** |
| **Does** | Copies bytes from `src` to `dst`, stopping after the first byte equal to `(unsigned char)c` has been copied, or after `n` bytes. |
| **Returns** | A pointer to the byte after the copied `c` in `dst`, or NULL if `c` was not among the first `n` bytes. |
| **Requires** | `n` bytes of room in `dst`. No overlap. |
| **Undefined if** | The objects overlap. |
| **Reads / writes** | At most `n` bytes each, one pass. Never reads past the stop byte. |
| **Terminated afterwards?** | With `c == '\0'`: only if it returned non-NULL. |
| **Reports truncation?** | Yes: NULL. |
| **Why it is shaped this way** | It is the one-pass bounded copy the `str` family lacks: `memccpy(dst, src, '\0', cap)` copies a string, reports truncation, returns the end, and does not read the rest of a long source. |
| **The usual mistake** | Forgetting to terminate after a NULL return: `dst[cap - 1] = '\0'`. |
| **Use instead, here** | `snprintf`, or your own loop. |
| **Use it for** | In C23 code, bounded chained copies. Know it exists. |

## Worked example: a path from two parts, without measuring twice

You have `dir` and `name` and a buffer `char path[64]`. Build `dir/name`, or fail.

The result's length is `strlen(dir) + 1 + strlen(name)`. It fits when that length is less
than 64, that is, when `ld + ln < 63`. So: measure both, decide, then copy with an end
pointer.

```c
size_t ld = strlen(dir), ln = strlen(name);
size_t room = sizeof path - 1;              /* 63: characters that fit, slash included */
if (ld >= room || ln >= room - ld)          /* ld + 1 + ln > 63, written without a sum */
    return -1;
char *p = stpcpy(path, dir);
*p++ = '/';
stpcpy(p, name);
```

With `dir = "usr"` and `name = "bin"`: `ld` is 3, `ln` is 3. The first test is `3 >= 63`,
false. The second is `3 >= 63 - 3`, that is `3 >= 60`, false. So it fits. `stpcpy` writes
`u s r \0` and returns `path + 3`. The slash overwrites that terminator at `path[3]` and `p`
becomes `path + 4`. The second `stpcpy` writes `b i n \0` at indices 4 to 7. Eight bytes
written, each once.

Why two comparisons instead of `ld + 1 + ln >= sizeof path`? Because a sum of `size_t` values
can wrap around to a small number. The first test rejects a `dir` that leaves no room for
the slash, and so guarantees that `room - ld` in the second is at least 1 and cannot wrap
below zero. Check the edge: `ld == 62` and an empty `name` give `0 >= 1`, false, and
`"<62 characters>/"` is 63 characters, which fits in 64 bytes exactly.

**The plausible wrong reading.** "`strcpy(path, dir); strcat(path, "/"); strcat(path, name);`
does the same thing after the same check." It produces the same bytes. But each `strcat`
first walks `path` from the start to find its end, so the characters of `dir` are read three
times. For three parts nobody will notice. In a loop over ten thousand parts the work is
proportional to the *square* of the output length (S04 e03 makes you wait for it). The check
was right; the copying was the wrong shape.

## Worked example: what is in the buffer?

`char buf[8]` holds `########`. Predict `strncpy(buf, "hello", 5)`, then `strlen(buf)`.

*How many bytes are written?* For `strncpy` that is always `n`: 5. *What are they?* The
source has 5 characters before its terminator. `strncpy` copies characters while it has not
written `n` bytes and has not reached the terminator; after `h e l l o` it has written 5, so
it stops. It never reached the terminator, and it would have had no room for it.

```
index:  0   1   2   3   4   5   6   7
buf:    h   e   l   l   o   #   #   #
```

`strlen(buf)` starts at index 0 and counts until it finds a NUL. Indices 5, 6, 7 hold `#`,
which is not NUL. It continues to index 8, which is outside `buf`. From there the behaviour
is undefined: in practice it returns some number of at least 8, or the program crashes, and
under ASan it is reported as a `READ`.

Now `strncpy(buf, "hello", 6)`: 6 bytes are written. Five characters, then the source's
terminator is reached with one byte still to write, so one NUL of padding: `hello\0##`. That
NUL is not "the terminator being copied" as a special act; it is the first byte of padding.
The effect is the same and the buffer is a string.

**The plausible wrong reading.** "`strncpy(buf, src, sizeof buf)` is the safe version: it
can never overflow `buf`." The second half is true: it writes exactly `sizeof buf` bytes.
But "safe" was about what happens *next*. If `src` has 8 or more characters, `buf` has no
terminator, and the overflow has only been moved from this line to the first line that reads
`buf` as a string, where it is harder to find. The call that would be safe is
`strncpy(buf, src, sizeof buf - 1); buf[sizeof buf - 1] = '\0';`, and at that point it is two
statements doing what one `snprintf` does, plus padding you did not want.

## Worked example: detecting truncation with `snprintf`

```c
char label[16];
int n = snprintf(label, sizeof label, "%s", name);
if (n < 0 || (size_t)n >= sizeof label) {
    /* did not fit: label holds the first 15 characters (if n >= 0) */
}
```

`snprintf` returns the number of characters the complete output has, not counting the
terminator, whether or not it all fitted. For `name = "configuration.yaml"` (18 characters)
it writes `configuration.y` and a terminator, 16 bytes, and returns 18.

Three details carry the weight. The test is `>=`, not `>`: a return of exactly 16 means 16
characters were needed, which need 17 bytes. The return type is `int`, and a negative value
is an error (an encoding error, or output longer than `INT_MAX`), so test `n < 0` *before*
converting. And the cast is to the unsigned type, after the negative case is excluded:
comparing an `int` directly with `sizeof label` converts the `int` to `size_t`, which turns
-1 into a huge number and happens to give the right branch for the wrong reason.

**The plausible wrong reading.** "`snprintf` returns the number of characters it wrote, so
`n == 15` tells me the buffer is full." It returns the number it *would have* written with
unlimited room. If it returned the count actually written, truncation would be undetectable:
a full buffer and a truncated one both hold 15 characters. The would-be length is exactly the
information that makes the function usable.

## What the tools say

**The compiler.** When gcc can see both the bound and the destination or source, it warns.
From `lesson-demo/capture-warn.txt` (gcc at `-O2 -Wall -Wextra`; the warnings are shown here
in source order, each with its source line):

```
warn.c:15:5: warning: ‘strncpy’ specified bound 12 equals destination size [-Wstringop-truncation]
   15 |     strncpy(m->name, name, sizeof m->name);
warn.c:21:5: warning: ‘strncpy’ output truncated before terminating nul copying 6 bytes from a string of the same length [-Wstringop-truncation]
   21 |     strncpy(code, "ABCDEF", 6);
warn.c:28:5: warning: ‘__builtin_memcpy’ forming offset [12, 23] is out of the bounds [0, 12] of object ‘small’ with type ‘char[12]’ [-Warray-bounds=]
   28 |     strcpy(small, "a name that is too long");
warn.c:35:32: warning: ‘%s’ directive output truncated writing 10 bytes into a region of size 8 [-Wformat-truncation=]
   35 |     return snprintf(label, 8, "%s", "abcdefghij");
```

Recognition rule: the flag in square brackets names the kind of defect, and the quoted
function is the call. "Specified bound equals destination size" is the usual mistake of the
`strncpy` card, word for word. The last one is not a defect by itself (`snprintf` truncated
safely); it tells you the result is known to be cut, and is silenced by using the return
value to handle that. clang has no `-Wstringop-truncation`; of these four it warns only
about the `strcpy` (`'strcpy' will always overflow; destination buffer has size 12, but the
source string has length 24`, under `-Wfortify-source`). And the compiler is silent as soon
as the size arrives through a function parameter, which is how real code is written. A
warning is evidence of a defect; no warning is evidence of nothing.

**AddressSanitizer.** An unterminated field read by `strlen`, from
`lesson-demo/capture-asan.txt` (long paths and build ids replaced by `...`):

```
==1510412==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x79b7f36e0020 ...
READ of size 13 at 0x79b7f36e0020 thread T0
    #0 0x7d97f548841b  (/usr/lib/libasan.so.8+0x8841b) ...
    #1 0x57d2ffc70dea in main .../lesson-demo/main.c:96
0x79b7f36e0020 is located 0 bytes after 16-byte region [0x79b7f36e0010,0x79b7f36e0020)
allocated by thread T0 here:
    #0 0x7d97f552c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1) ...
    #1 0x57d2ffc738cf in ds_malloc_at .../lib/dsalloc.c:216
    #2 0x57d2ffc70c5c in main .../lesson-demo/main.c:91
```

Read it in this order. **Line 2, `READ`**: nothing was overwritten; something was read that
should not have been. With strings, a READ overflow almost always means a missing
terminator. **`of size 13`**: the reader touched 13 bytes, twelve inside the field and one
past it. **Frame `#0`** is inside the sanitizer's library: ASan replaces `strlen`, `strcpy`
and their relatives with checking versions, and that is where the access is detected. (With
clang the frame is named, `in strlen`.) **Frame `#1`** is the first line of your code, the
call to `strlen`. **"0 bytes after 16-byte region"**: the bad byte is the one immediately
after a 16-byte heap block, and **"allocated by"** says which block: the first frame there
that is your code (`main.c:91`; `ds_malloc_at` is the course's allocation counter) is the
`malloc` of the record. So a
string reader ran off the end of the record, and the record's last field has no terminator.
The line to fix is none of the ones in the report: it is the copy *into* that field. Search
for writes to the field, not for the reader.

**Nothing at all.** The overflow from one struct member into the next (see *What ASan cannot
see*) produces no report from ASan, UBSan or, usually, the compiler. e07 fails only because
its harness checks the neighbouring field.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| length vs. capacity | Length counts characters before the terminator. Capacity counts the buffer's bytes. A string fits when `len < cap`. |
| the `n` of `strncpy` vs. the size of `snprintf` | `strncpy` writes exactly `n` bytes and may use all of them for characters. `snprintf` writes at most `size` bytes and always keeps one for the terminator. |
| `strncpy` padding vs. terminating | `strncpy` never "adds a terminator". It pads with NULs when the source is short. When there is nothing to pad, there is no NUL. |
| truncated vs. unterminated | Truncated: a shorter but valid string (`snprintf`). Unterminated: not a string at all (`strncpy` with a long source). The first loses data; the second is an over-read waiting for a reader. |
| "wrote" vs. "would have written" | `snprintf` and `strlcpy` return the length the full result needs, not what was stored. That is what makes truncation detectable. |
| returns `dst` vs. returns the end | `strcpy`/`strncpy` return what you passed in. `stpcpy`/`stpncpy` return where to continue. Only the second kind chains without rescanning. |
| where the tool points vs. where the defect is | An over-read is reported in the reader. The defect is the copy that left the field unterminated. |
| overflow out of an object vs. within one | ASan reports the first. A write from one struct member into the next stays inside the object and is not reported. |

## `man` pointers

- `man 3 strcpy`: SYNOPSIS (the `restrict` qualifiers), RETURN VALUE, CAVEATS. On Linux the
  page covers `strcpy` and `strcat`; on macOS one page covers `strcpy`, `stpcpy`, `strncpy`
  and `stpncpy`.
- `man 3 strncpy`, `man 3 stpcpy`, `man 3 stpncpy` (Linux; on macOS all are `man 3 strcpy`):
  DESCRIPTION for the padding, CAVEATS for "a null-padded character sequence, not a string",
  STANDARDS for ISO C versus POSIX.
- `man 7 string_copying` (Linux only): one page comparing every copy function.
- `man 3 snprintf` (`man 3 printf`): RETURN VALUE, the paragraph about truncated output.
- `man 3 strlcpy`: exists on both systems even though the course build cannot call it.

## Check yourself

1. `char b[6];` Which of `"hello"`, `"hello!"`, `""` fit in `b`? State the test you applied.
2. All 8 bytes of a `########` buffer after `strncpy(buf, "ab", 5)`.
3. After `strncpy(buf, "abcdefgh", 8)` on an 8-byte `buf`, what does `strlen(buf)` do?
4. What do `stpcpy(buf, "")` and `stpncpy(buf, "abc", 3)` return?
5. `snprintf(buf, 4, "%s", "hello")`: the bytes written and the return value.
6. Write the condition that is true exactly when `snprintf(dst, cap, "%s", s)` truncated or
   failed, given `int n` holding its result.
7. A program copies a file name with a truncating copy and then deletes the file. Name the
   defect and the policy that removes it.
8. Why does ASan not report `strcpy(a.user, long_name)` when `user` is the first member of a
   larger struct?
9. Why does `strlcpy` read a megabyte to copy 7 bytes of a megabyte string?

<details><summary>Answers</summary>

1. `"hello"` (5 < 6) and `""` (0 < 6) fit; `"hello!"` (6 < 6 is false) does not. Test:
   `len < cap`.
2. `ab\0\0\0###`: two characters, three NULs of padding to make 5 bytes, three untouched.
3. Undefined behaviour: there is no NUL in `buf`, so `strlen` reads `buf[8]` and beyond.
4. `buf` (the terminator is at index 0), and `buf + 3` (no NUL was written; the result
   equals `dst + n`).
5. `hel\0` (4 bytes); returns 5.
6. `n < 0 || (size_t)n >= cap`.
7. The truncated name can be the name of a *different*, existing file. Use an all-or-nothing
   copy and treat "does not fit" as an error.
8. The write stays inside the struct, which is a single object to ASan; no redzone is
   crossed.
9. Its return value is `strlen(src)`, so it must walk the whole source to compute it.

</details>

## Takeaways

- A string fits a buffer when `len < cap`; every bounded copy is that comparison plus a policy.
- `strncpy` writes exactly `n` bytes and terminates only by accident of the source's length; it is a field filler, not a bounded `strcpy`.
- A truncating copy whose return value is ignored is a silent data-corruption bug, and for identifiers the right policy is to refuse.
- `snprintf(dst, cap, "%s", src)` is the standard library's `strlcpy`: always terminated, truncation visible as `n >= cap`.
- Functions that return the end compose in linear time; functions that return the start force a rescan.
- A report about a string over-read names the reader; fix the writer that dropped the terminator.
- "ASan is clean" does not cover overflow between members of one struct.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-my-strcpy-stpcpy/` | `[WRITE]` | S | `my_strcpy` and `my_stpcpy`, tested against libc |
| e02 | `e02-my-strncpy/` | `[WRITE]` | S | `my_strncpy` and `my_stpncpy`: exactly `n` bytes, padding, no terminator when full |
| e03 | `e03-my-strlcpy/` | `[WRITE]` | S | `my_strlcpy`: always terminated, returns `strlen(src)` |
| e04 | `e04-strncpy-predict/` | `[PREDICT]` | | the 8 bytes of a buffer and the return value after ten copy calls |
| e05 | `e05-unterminated-name/` | `[BUG]` | | a full-width `strncpy` into a record field, found from an over-read report |
| e06 | `e06-copy-policy/` | `[DESIGN]` | M | three bounded copies with three policies; contracts first |
| e07 | `e07-struct-neighbour/` | `[BUG]` | | `strcpy` overflows one struct member into the next; ASan is silent |
| e08 | `e08-copy-lookup/` | `[LOOKUP]` | | nine facts about the copy functions, from the manual pages, with quotes |

## Drills

| # | Folder | Skill | Target |
|---|---|---|---|
| r01 | `drills/r01-bounded-copy-reps/` | a bounded copy four ways, each reporting truncation | 12 min |
| r02 | `drills/r02-fixed-field-reps/` | write, measure, read and compare a NUL-padded field | 12 min |
| r03 | `drills/r03-stpcpy-chain-reps/` | build a string from parts with an end pointer | 10 min |
