# S04: Concatenation

## Organizer

Appending is copying plus one question: where does the text already end? The C library
answers it in the most expensive way possible — by searching — and that single decision, made
in 1972 and frozen into `strcat`'s interface, is why appending in a loop is the classic C
performance bug and why every modern alternative takes or returns a position.

What changes in your thinking: you stop seeing an append as "add this to that" and start
seeing three separate questions, each of which the function you choose either answers or
silently does not. **How much room is left?** (arithmetic you must do, or pass to someone who
will). **How much was actually written?** (two of the four functions refuse to say). **Where
does the result end now?** (keep it, or pay to find it again next time).

You need S03 (bounded copies, the `len < cap` test, the `snprintf` return-value contract) and
S01 (`size_t` arithmetic and the wrap at zero). Memory course M07 is assumed for the heap
string in e07.

After this unit you can:

- say what each of `strcat`, `strncat`, `snprintf` and the `strlcat` contract writes, returns
  and reports, and choose between them from the data rather than from the name;
- write the correct third argument for `strncat` and explain each of its three terms;
- recognise a quadratic append on sight and fix it without changing the function's interface;
- build an appender that cannot overflow, cannot forget to terminate and reports truncation
  once at the end;
- compute room and required sizes in `size_t` with no expression that can wrap;
- build a heap string with a measuring pass and a writing pass that cannot disagree, and keep
  it all-or-nothing under forced allocation failure.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| `strcat` | `<string.h>` | ISO C | below | append the whole string; no bound; returns `dst` |
| `strncat` | `<string.h>` | ISO C | below | append at most `n` characters **of the source**, then always a terminator |
| `strlcat` | `<string.h>` | BSD; **not in the portable set** | below | append within the destination's size; returns the length it wanted |
| `snprintf` with `"%s%s"` | `<stdio.h>` | ISO C | S11 | rebuild bounded; returns the length it wanted |
| `vsnprintf` | `<stdio.h>` | ISO C | S11 | the same from a `va_list`: how a wrapper formats (e05) |

`strlcat` is not declared under this course's feature macros (SPEC section 10), so you write
it in e02. The two `printf`-family entries get their cards in S11; here they appear as the
appending tools they are in practice.

## Foundations

### What appending costs

`strcat(dst, src)` must find `dst`'s terminator before it can write a byte, and the only way
to find it is to walk from the start. The Linux manual page states the function as an
equivalence, which is the clearest possible statement of its cost:

```
strcat(dst, src)   is equivalent to   stpcpy(strnul(dst), src), dst
```

`strnul` is "pointer to the terminator" — the walk — and `stpcpy` is the copy. So one append
is `O(strlen(dst) + strlen(src))`, and a loop that appends `n` pieces of `w` characters walks

```
0 + w + 2w + ... + (n-1)w   ≈   w·n²/2
```

bytes to produce `w·n` bytes of output. The useful work is linear; the searching is
quadratic. `lesson-demo` measures it on this machine, doubling `n` four times:

```
what it costs to append n words of 4 characters, two ways

       words     strcat loop     end pointer  ratio
       -----     -----------     -----------  -----
       10000        0.0012 s        0.0000 s      45x
       20000        0.0063 s        0.0001 s     112x
       40000        0.0253 s        0.0001 s     186x
       80000        0.1034 s        0.0006 s     185x
```

Nothing in the implementation can fix this. glibc's `strcat` is a vectorised `strlen` plus a
vectorised `memcpy`, so each call is as fast as it can be; the ratio still quadruples with
every doubling, because the number of walks grows with the square. The interface is the
problem: a function told only where the string *starts* must find where it ends.

The fix is always the same, and it is the subject of e03, e05 and drill r01: **keep the
position you already have.** A program that is appending already knows how long the result is
so far — it usually needs that number anyway — and the moment it keeps it, appending becomes
`O(what you append)`.

### The three reports

Four ways to append, and they differ less in what they write than in what they tell you:

```
appending "cd" and "cdefghij" to "ab" in an 8-byte buffer

  call                               buffer after      ret  note
  ----                               ------------      ---  ----
  strcat(buf, "cd")                  abcd\0###         buf  no bound at all
  strncat(buf, "cdefghij", 3)        abcde\0##         buf  3 characters, then a terminator
  strncat(buf, "cdefghij", 5)        abcdefg\0         buf  5 characters: the buffer is full
  snprintf(buf, 8, "%s%s", ...)      abcd\0###           4  rebuilds; returns the length
  snprintf(buf, 8, ... too long)     abcdefg\0          10  truncated: ret >= 8
  strlcat-style append of "cd"       abcd\0###           4  appends; returns the length
  strlcat-style, too long            abcdefg\0          10  truncated: ret >= 8
```

| | Writes | Returns | Truncation visible? | Bounded by |
|---|---|---|---|---|
| `strcat(dst, src)` | all of `src` + terminator | `dst` | no — it overflows instead | nothing |
| `strncat(dst, src, n)` | ≤ `n` characters + terminator | `dst` | no | the **source** |
| `snprintf(dst, cap, ...)` | ≤ `cap` bytes, always terminated | the length it **wanted** | yes: `ret >= cap` | the destination |
| `strlcat(dst, src, size)` | the same bytes | the length it **wanted** | yes: `ret >= size` | the destination |

Read the "Bounded by" column twice. `strncat`'s `n` is a count of characters taken **from the
source**; it has nothing to do with how large `dst` is. The Linux page gives the destination
requirement as a formula — the buffer must be at least
`strlen(dst) + strnlen(src, ssize) + 1` — and the `+ 1` is the terminator `strncat` always
writes. So `strncat` can write `n + 1` bytes, and `strncat(dst, src, sizeof dst)` is a buffer
overflow (e04).

The correct third argument is the room left for characters:

```c
strncat(dst, src, cap - strlen(dst) - 1);
```

`cap` is the buffer's size, `strlen(dst)` is what is there, and the `- 1` is the terminator.
Three terms, and the expression is undefined for `cap == 0`, so a guard comes first.

### Room, and the arithmetic that must not wrap

Every append begins with the same question, and it has one correct form:

```
room  =  cap - len - 1          /* characters that fit, terminator reserved */
fits  =  add <= cap - len - 1   /* NOT len + add + 1 <= cap                 */
```

Both expressions are only safe once `len < cap` is known, which is why a bounded appender
tests `cap == 0` and `len >= cap` first. The rule for the rest: **move a term across rather
than adding two unbounded values.** `b > SIZE_MAX - a` asks whether `a + b` would wrap without
performing it; `a + b > SIZE_MAX` performs the wrap and then tests the wrong number. Drill r02
is this arithmetic at the extremes of `size_t`.

A sum of part lengths needs the same care. Two parts and a terminator need `a + b + 1`; three
parts with two one-character separators need `a + b + c + 3`; and each addition gets a wrap
test before it happens. When no answer is representable, report it — 0 is a good signal,
because it is not a legal size for a buffer that must hold even the empty string.

### An appender keeps its place

Put those two ideas together and you get the shape the rest of this course uses:

```c
typedef struct { char *buf; size_t cap, len; int truncated; } Cursor;
```

`len` is where the next character goes (so no walk) and `truncated` is **sticky** (so the
caller checks once, at the end, instead of after every call). An append becomes:

```
room = cap - len - 1
keep = min(strlen(s), room)
memcpy(buf + len, s, keep); len += keep; buf[len] = '\0';
if (keep < strlen(s)) truncated = 1;
```

Why sticky? Because once anything has been dropped the buffer no longer holds what the caller
asked for, and a later append that fits does not change that. A flag cleared by success would
tell a caller checking at the end that all was well.

The same shape with two pointers — `p` for "where the next character goes", `end` for "one
past the buffer" — is drill r01, and it is what the Linux `string_copying(7)` page's
`stpecpy` sketch proposes. e05 builds the struct version, including a `vsnprintf` wrapper,
because formatted appending is where a cursor earns its keep.

### Building on the heap: measure, then write

When the result goes on the heap, the length must be known **before** the allocation, so the
function runs the same logic twice: once counting, once writing. The failure mode is the two
passes disagreeing, and the fix is not care but structure — one function that either counts
or writes:

```c
static size_t build(char *out, const char *const *parts, size_t n);  /* out == NULL: count */
```

With the rule in one place, the measuring pass cannot drift from the writing pass when the
rule changes. e07 is that shape, under forced allocation failure: on any failure the function
returns NULL having allocated nothing, which the fault sweep checks by failing each
allocation in turn.

## Function reference

### `strcat`

```c
#include <string.h>
char *strcat(char *restrict dst, const char *restrict src);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.3.1) |
| **Does** | Finds `dst`'s terminator and copies `src`, including its terminator, from there. |
| **Returns** | `dst`. Never an error indication, never a length. |
| **Requires** | `dst` is a string with room for `strlen(dst) + strlen(src) + 1` bytes — the Linux page states that formula. `src` is a string. No overlap. |
| **Undefined if** | The buffer is too small; either argument is not a string; the objects overlap. |
| **Reads / writes** | Reads all of `dst` and all of `src`; writes `strlen(src) + 1` bytes. **Two passes over `dst`'s length per call**, counting the walk. |
| **Terminated afterwards?** | Yes. |
| **Reports truncation?** | It never truncates. It overflows. |
| **Why it is shaped this way** | It is `strcpy`'s sibling from a time when buffers were sized by convention. Returning `dst` rather than the new end was the choice that makes appending in a loop quadratic. |
| **The usual mistake** | Using it in a loop (e03), and using it at all on data whose length the program does not control. |
| **Use instead** | An end pointer or a cursor (e05); `snprintf` with its return value checked; the `strlcat` contract (e02). |
| **Use it for** | One append into a buffer you have just measured against, where the alternative would be less clear. Even then, `memcpy` at a known offset is clearer. |

```
dst: a b c \0 # # # #     strcat(dst, "def")  ->  a b c d e f \0 #     returns dst
```

### `strncat`

```c
#include <string.h>
char *strncat(char *restrict dst, const char *restrict src, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.3.2) |
| **Does** | Finds `dst`'s terminator, appends at most `n` characters of `src` (stopping early at `src`'s terminator), then **always** writes one terminator. |
| **Returns** | `dst`. |
| **Requires** | `dst` is a string in a buffer of at least `strlen(dst) + strnlen(src, n) + 1` bytes. `src` is a string **or** has `n` readable bytes. No overlap. |
| **Undefined if** | The buffer is too small — which is what `strncat(dst, src, sizeof dst)` guarantees. |
| **Reads / writes** | Reads all of `dst`, and at most `n` bytes of `src`. Writes between 1 and `n + 1` bytes. |
| **Terminated afterwards?** | Always. |
| **Reports truncation?** | No. The return value is `dst` whether the source was appended whole, clipped or empty. |
| **Why it is shaped this way** | It is `strncpy`'s sibling in name only: `strncpy`'s `n` bounds the destination, `strncat`'s bounds the source. The asymmetry is historical and has caused a great many overflows. |
| **The usual mistake** | `strncat(dst, src, sizeof dst)` or `strncat(dst, src, cap)`. The only correct value is `cap - strlen(dst) - 1` (e04). |
| **Use instead** | `snprintf(dst + len, cap - len, "%s", src)`; the `strlcat` contract; a cursor. |
| **Use it for** | Appending a bounded number of characters from a source that may not be terminated — a fixed-width field, for instance. That, and nothing else. |

```
dst: a b c \0 # # # #   strncat(dst, "defgh", 3)  ->  a b c d e f \0 #   (3 chars + 1)
dst: a b c \0 # # # #   strncat(dst, "de", 5)     ->  a b c d e \0 # #   (stops at src's end)
dst: a b c \0 # # # #   strncat(dst, "defgh", 0)  ->  a b c \0 # # # #   (just the terminator)
```

### `strlcat`

```c
size_t strlcat(char *dst, const char *src, size_t size);   /* BSD */
```

| | |
|---|---|
| **Standard** | None in C17 or POSIX.1-2008. OpenBSD 2.4, the other BSDs, macOS; glibc 2.38; POSIX.1-2024. **Not in the portable set**: you write it in e02. |
| **Does** | Appends as much of `src` as fits in the `size`-byte buffer, and always terminates when anything is written. |
| **Returns** | The length of the string it **tried** to create: `strlen(dst) + strlen(src)` normally, or `size + strlen(src)` when `dst` holds no terminator within `size`. |
| **Requires** | `src` is a string; `dst` has `size` readable and writable bytes. `size` may be 0; `dst` need **not** be terminated. |
| **Undefined if** | `src` is not terminated. |
| **Reads / writes** | Reads at most `size` bytes of `dst` and **all** of `src`, every time, even when nothing fits. Writes at most `size` bytes. |
| **Terminated afterwards?** | Yes, whenever `size > 0` and `dst` held a terminator. |
| **Reports truncation?** | Yes: `ret >= size`, and `ret + 1` is the size that would have been enough. |
| **Why it is shaped this way** | It was designed so that a caller can pass `sizeof dst` and test one comparison, instead of computing room and checking nothing. |
| **The usual mistake** | Ignoring the return value, which turns "truncate and report" into "truncate silently". |
| **Use instead, here** | `snprintf(dst + len, cap - len, "%s", src)`: the same bytes and the same number, as an `int`. |
| **Use it for** | Bounded appends of text where truncation is acceptable **and checked**. Note the cost: it reads the whole source even when none of it fits, which is glibc's long-standing objection to it. |

```
dst (size 8): a b \0 # # # # #   append "cd"        ->  a b c d \0 # # #   returns 4
dst (size 8): a b \0 # # # # #   append "cdefghij"  ->  a b c d e f g \0   returns 10  (>= 8)
dst (size 8): a b c d e f g h    append "xy"        ->  unchanged          returns 10  (no terminator)
```

## Worked example: appending three pieces into a fixed buffer

A label of at most 15 characters, built from a kind, a separator and some text, in
`char buf[16]`. Four ways, and only two of them tell you what happened.

```c
/* 1. strcat: no bound at all. One long `text` and the stack is gone. */
strcpy(buf, kind); strcat(buf, ": "); strcat(buf, text);

/* 2. strncat: bounded, and the bound is easy to get wrong. */
strncat(buf, text, sizeof buf - strlen(buf) - 1);

/* 3. snprintf: bounded, reports, and rebuilds the whole string each time. */
int n = snprintf(buf, sizeof buf, "%s: %s", kind, text);
if (n < 0 || (size_t)n >= sizeof buf) { /* truncated */ }

/* 4. a cursor: bounded, reports once, and never rescans. */
Cursor c; cur_init(&c, buf, sizeof buf);
cur_puts(&c, kind); cur_puts(&c, ": "); cur_puts(&c, text);
if (cur_truncated(&c)) { /* truncated */ }
```

With `kind = "warn"` and `text = "disk"`, all four produce `warn: disk` and 10 characters.
With `text = "disk is full and the log is too"`, the first overflows, the second truncates
silently, and the third and fourth report.

Count the walks. Version 1 walks `buf` twice (once per `strcat`); version 2 walks it twice
more (once for `strlen`, once inside `strncat`); version 3 rebuilds from the start, so it
re-copies `kind` and the separator on every call; version 4 walks nothing at all. For three
pieces none of this matters. The point is that versions 1 to 3 get slower as the buffer fills
and version 4 does not, which is the difference between a loop that scales and e03.

**The plausible wrong reading.** "Version 3 is the safe one, so `snprintf` is the answer and
the cursor is an optimisation." `snprintf` *is* safe, and it is the right tool for a single
formatted write. Used as an appender it has a property worth noticing: it rebuilds the whole
string each time, so appending `n` pieces re-formats everything written so far — quadratic
again, for a different reason. The honest `snprintf` append is
`snprintf(buf + len, cap - len, ...)` with `len` maintained by the caller — at which point the
caller is keeping a cursor and may as well say so.

## Worked example: how large must the destination be?

The Linux pages state both formulas, and applying them is the whole of e08:

```
strcat:   strlen(dst) + strlen(src) + 1
strncat:  strlen(dst) + strnlen(src, n) + 1
```

With `dst` holding `"abc"` (3 characters), `src` being `"defgh"` (5) and `n` 2:

- `strcat` needs 3 + 5 + 1 = **9** bytes.
- `strncat` needs 3 + 2 + 1 = **6** bytes, because only two characters of the source are
  taken.

Both formulas end in `+ 1`, and in both cases it is the terminator. For `strcat` it comes from
the source; for `strncat` the function writes it itself, which is why `strncat` writes up to
`n + 1` bytes and why the formula uses `strnlen(src, n)` rather than `n`: a short source costs
less room, not more.

Now invert the `strncat` formula. A caller has a buffer of `cap` bytes holding `strlen(dst)`
characters and wants to know the largest `n` that cannot overflow:

```
strlen(dst) + n + 1 <= cap      =>      n <= cap - strlen(dst) - 1
```

which is the expression of e04, derived rather than remembered. And note what it is not: it
is not `cap`, and it does not simplify to anything shorter.

**The plausible wrong reading.** "`strncat(dst, src, sizeof dst)` is at worst wasteful — it
will just stop when the buffer is full." Nothing stops it. `strncat`'s `n` never looks at the
destination; it is a bound on how much of the source to read. With `sizeof dst` as the bound
and a source that long, the function appends `sizeof dst` characters *after* whatever was
already there and then writes a terminator — up to `sizeof dst + 1` bytes past the existing
contents. Both course compilers have a diagnostic for the literal form (see below), and both
fall silent when the size arrives through a parameter.

## What the tools say

**The compiler, when it can see the sizes.** `lesson-demo/warn.c` has three appends. From
`lesson-demo/capture-warn.txt` (gcc at `-O2 -Wall -Wextra -Wpedantic`):

```
warn.c: In function ‘add_tag’:
warn.c:12:5: warning: ‘strncat’ specified bound 8 equals destination size [-Wstringop-truncation]
   12 |     strncat(buf, src, sizeof buf);
      |     ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
warn.c:12:5: warning: ‘strncat’ specified bound 8 equals destination size [-Wstringop-overflow=]
warn.c: In function ‘greet’:
warn.c:21:5: warning: ‘__builtin_memcpy’ writing 8 bytes into a region of size 3 [-Wstringop-overflow=]
   21 |     strcat(small, ", world");
      |     ^~~~~~~~~~~~~~~~~~~~~~~~
warn.c:19:10: note: at offset 5 into destination object ‘small’ of size 8
   19 |     char small[8] = "hello";
      |          ^~~~~
```

and the same file through clang, from `capture-warn-clang.txt`:

```
warn.c:12:23: warning: the value of the size argument in 'strncat' is too large, might lead to a buffer overflow [-Wstrncat-size]
   12 |     strncat(buf, src, sizeof buf);
      |                       ^~~~~~~~~~
warn.c:12:23: note: change the argument to be the free space in the destination buffer minus the terminating null byte
   12 |     strncat(buf, src, sizeof buf);
      |                       ^~~~~~~~~~
      |                       sizeof(buf) - strlen(buf) - 1
```

Both compilers catch `strncat(buf, src, sizeof buf)`, and clang's message goes further than a
diagnosis: it prints the expression you should have written. gcc catches the `strcat` into a
buffer it can see is too small; clang does not. Neither says anything about the third
function, an `snprintf` that truncates — because truncating is what `snprintf` is *for*.

As always, the diagnostics need the size as a constant at the call. e04's defective call is
one line inside a helper whose arguments are parameters, and both compilers are silent about
it. **A warning is evidence of a defect; no warning is evidence of nothing.**

**AddressSanitizer, for the overflow.** `make asan ARGS=overflow` in `lesson-demo` makes the
`strncat` call with the destination's size as the bound; from `capture-overflow.txt` (paths
and build ids shortened):

```
==1944260==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x6fc0413d0048 at pc 0x73c04432466a bp 0x7fff342fe590 sp 0x7fff342fdd38
WRITE of size 5 at 0x6fc0413d0048 thread T0
    #0 0x73c044324669 in strncat (/usr/lib/libasan.so.8+0x124669)
    #1 0x578f93337549 in cat_n .../lesson-demo/main.c:21
    #2 0x578f93337af3 in main .../lesson-demo/main.c:71
```

`WRITE of size 5` is the whole range the interceptor was about to write — five bytes, not one,
because ASan checks the complete write before performing it. Frame `#0` is ASan's replacement
for `strncat`, frame `#1` is the helper that passed the argument, and the line to fix is in
the frame above that, where the argument was chosen.

**Nothing at all.** The quadratic append of e03 produces no report from any tool: it is
correct code. It is caught by a **cost budget** — the harness times a linear reference pass
and requires the function to be within fifty of it — which is the only kind of check that can
see a defect of this shape. The budget is written as a multiple of a measurement rather than a
number of seconds so that it means the same thing on a fast laptop and a loaded CI machine.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| `strncpy`'s `n` vs. `strncat`'s `n` | `strncpy` writes exactly `n` bytes into the destination. `strncat` takes at most `n` characters from the source and writes up to `n + 1` bytes. |
| `strncat`'s bound vs. the destination's size | `strncat(dst, src, cap)` is an overflow. The only correct bound is `cap - strlen(dst) - 1`. |
| appending vs. rebuilding | `strcat` and `strlcat` append at the end. `snprintf(dst, cap, "%s%s", old, new)` rebuilds from the start, re-copying what was there. |
| "it fits" vs. "it all fitted" | `strncat` and `strcat` report neither. `snprintf` and `strlcat` return the length they wanted, so `ret >= cap` is the test. |
| room vs. capacity | Room is `cap - len - 1`: what still fits, terminator reserved. Capacity is the buffer. |
| `a + b > SIZE_MAX` vs. `b > SIZE_MAX - a` | The first performs the wrap and tests the wrong number; the second asks the question. |
| a sticky truncation flag vs. a per-call result | A flag cleared by a later success tells a caller checking at the end that nothing was lost. |
| correct vs. fast enough | e03's join produces every right answer and fails its budget by 300×. No sanitizer can see that. |

## `man` pointers

- `man 3 strcat` (shared with `strcpy` on Linux): DESCRIPTION for the size formula
  `strlen(dst) + strlen(src) + 1` and for the `stpcpy(strnul(dst), src)` equivalence,
  RETURN VALUE for "these functions return dst".
- `man 3 strncat`: DESCRIPTION for "at most ssize non-null bytes ... followed by a null
  character" and for the buffer formula with `strnlen(src, ssize)`.
- `man 3 strlcat` (both platforms, though the function is not callable here): STANDARDS, which
  says "None", and CAVEATS, which records glibc's objection — it reads the whole source even
  when truncating.
- `man 3 snprintf` (`man 3 printf`): RETURN VALUE, the paragraph about output being truncated.
- `man 7 string_copying` (Linux only): the whole family in one page, with the `stpecpy` and
  `strtcpy` sketches that take an end pointer.

## Check yourself

1. `char buf[8]` holds `"abc"`. What is the correct third argument for
   `strncat(buf, src, ?)`, and how many bytes may the call then write?
2. `strncat(dst, src, 4)` with a source of two characters: how many bytes are written?
3. A loop appends 10,000 pieces of 5 characters with `strcat`. How many bytes are walked, to
   within a factor of two, and how many are written?
4. Which of the four appending idioms tell the caller that the result did not fit, and what
   exactly do they return?
5. Write the test "will `add` more characters fit in a buffer of `cap` bytes holding `len`?"
   so that no expression can wrap.
6. Why must a truncation flag be sticky?
7. `path_join` measures the result, allocates, and writes it. What goes wrong if the two
   passes use different code, and what is the fix?
8. A cursor's `cur_printf` calls `vsnprintf`. What size does it pass, and what are the two
   ways `vsnprintf` reports that something was dropped?

<details><summary>Answers</summary>

1. `sizeof buf - strlen(buf) - 1`, which is `8 - 3 - 1 = 4`. The call may then write up to
   `4 + 1 = 5` bytes, filling the buffer exactly.
2. Three: the two characters of the source, then the terminator. The bound is "at most".
3. Output is 50,000 bytes. The walks are about `5 · n²/2 = 2.5 × 10⁸` bytes — four thousand
   times the output.
4. `snprintf` returns the length the complete result needed, as an `int` (negative on an
   encoding error); the `strlcat` contract returns the same number as a `size_t`. Both mean
   truncation when the value is `>= ` the size. `strcat` and `strncat` return `dst`.
5. `cap != 0 && len < cap && add <= cap - len - 1`.
6. Because the buffer no longer holds what the caller asked for, and a later append that fits
   does not change that. A flag cleared by success makes a check at the end useless.
7. The allocation is sized by one rule and filled by another, so the result is one or two
   bytes too large or too small — the second of which is a heap overflow at the terminator.
   The fix is one function that either counts or writes, chosen by a null destination.
8. `cap - len`, which includes the terminator's byte (`vsnprintf` reserves its own). It
   reports trouble with a negative return (an encoding error) or a return `>= ` the size it
   was given (truncation), and the negative case must be tested before any cast to `size_t`.

</details>

## Takeaways

- An append that is not told where the string ends must search for it, and a loop of such
  appends is quadratic however fast the library is.
- `strncat`'s `n` bounds the source, not the destination: the only correct value is
  `cap - strlen(dst) - 1`.
- Only two appending idioms report truncation, and both do it by returning the length they
  *wanted*.
- Room is `cap - len - 1`, and the test for it must move a term across rather than add two
  unbounded values.
- An appender that keeps its position and a sticky truncation flag turns `n` checks into one
  and `O(n²)` into `O(n)`.
- A heap builder measures and writes with the same code, or the two passes will disagree.
- A correctness suite cannot see a quadratic append; a cost budget measured against a linear
  reference can.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-my-strcat-strncat/` | `[WRITE]` | S | both appends, with the `n + 1` byte count |
| e02 | `e02-my-strlcat/` | `[WRITE]` | S | the BSD append, including an unterminated destination |
| e03 | `e03-shlemiel/` | `[BUG]` | | 60,000 appends, right answers, 300× over budget |
| e04 | `e04-strncat-size/` | `[BUG]` | | `strncat(dst, src, cap)`: a bound on the wrong string |
| e05 | `e05-append-cursor/` | `[WRITE]` | M | an allocation-free appender with a sticky truncation flag |
| e06 | `e06-room-predict/` | `[PREDICT]` | | twelve appends: the bytes, the return value, the room left |
| e07 | `e07-path-join/` | `[FAULT]` | M | heap path joining, all-or-nothing under every `FAIL_AT` |
| e08 | `e08-strcat-lookup/` | `[LOOKUP]` | | nine facts from the pages, including both size formulas |

## Drills

| # | Folder | Skill | Target |
|---|---|---|---|
| r01 | `drills/r01-append-reps/` | append a string, a character and a number at an end pointer | 10 min |
| r02 | `drills/r02-room-math-reps/` | room, fits, and sizes for two and three parts, without wrapping | 10 min |
| r03 | `drills/r03-join-reps/` | join n strings into a fixed buffer, three ways | 12 min |
