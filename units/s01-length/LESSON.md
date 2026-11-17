# S01: Length

## Organizer

`strlen` is the function C programmers call most and think about least. It takes a pointer,
reads forward until it finds a zero byte, and returns how many bytes that took. There is no
bound, no error return, and no way for it to tell whether the bytes it was given were ever a
string. Everything in this unit follows from that one sentence.

What changes in your thinking: you stop treating "the length" as a property a buffer has and
start treating it as a *measurement someone performs*, with a cost, a precondition, and a type.
The cost is a pass over the data, so a length in a loop condition is a loop that reads the
string once per iteration. The precondition is the terminator, so a length is a question you
may not ask about a buffer that has none. And the type is `size_t`, which is unsigned, so
`length - 1` on an empty string is not -1 but the largest number the machine has.

You need S00 (the terminator convention, `sizeof` against `strlen`, `char` signedness). D00
from the data structures course helps for the cost argument, and is not required.

After this unit you can:

- say what `strlen` reads, what it costs, and what it requires of its argument, and recognise
  the calls where that requirement is not met;
- measure bytes that are not a string, with `strnlen` or `memchr`, and read the result's
  double meaning (a length, or "no terminator in there");
- write `strlen` and `strnlen` from a blank file, correct for the empty string, for an
  embedded NUL and for `maxlen == 0`;
- predict the result of any `size_t` subtraction at zero, and write loops that have no such
  subtraction in them;
- store a length in a narrower type with a range check at the conversion, and say why no tool
  reports the missing check;
- read a heap over-read report back to the allocation that was one byte too small.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| `strlen` | `<string.h>` | ISO C | below | count the bytes before the terminator; requires one |
| `strnlen` | `<string.h>` | POSIX.1-2008 | below | the same count over at most `maxlen` bytes; requires nothing |
| `memcpy` | `<string.h>` | ISO C | S02 | copy exactly `n` bytes; no terminator involved |
| `memcmp` | `<string.h>` | ISO C | S02 | compare exactly `n` bytes as `unsigned char` |
| `sizeof` | — (an operator) | ISO C | S00 | the storage of a type, at compile time |

`memcpy` and `memcmp` appear in e02 because a field reader needs them; their cards are in S02,
where they are taken apart. This unit's own two functions are below, and the pair is the whole
subject: one trusts the convention, one takes a bound.

## Foundations

### A length is not a size

From `string_copying(7)`, the vocabulary this course uses:

| Word | Means | Measured by |
|---|---|---|
| **length** | the number of non-zero bytes before the terminator | `strlen`, `strnlen` |
| **size** | the whole buffer the bytes sit in | `sizeof`, or a `cap` parameter |

A string of length `len` occupies `len + 1` bytes, so it fits in a buffer of size `cap`
exactly when `len < cap`. The two numbers are never interchangeable, and this is what the
course's `lesson-demo` prints for five objects (`make run` in `units/s01-length/lesson-demo`):

```
length against size, and what each function may be asked

  object                     bytes                    strlen   strnlen
  ------                     -----                    ------   -------
  char a[] = "hello";        hello\0                       5         5
  char e[] = "ab\0cd";       ab\0cd\0                      2         2
  char f[8] = "hi";          hi\0\0\0\0\0\0                2         2
  char field4[4] = {chars};  A123                   undefined         4

  strnlen is given the object's size; strlen is given nothing but the pointer.
```

Read the last row twice. `field4` holds four characters in four bytes, so there is no
terminator, so there is no length for `strlen` to return — and `strnlen`, given the object's
size as its bound, answers 4. Both functions are correct; only one of them was asked a
question it could answer.

### What `strlen` costs and what it trusts

C17 7.24.6.3, in full: "The `strlen` function computes the length of the string pointed to by
`s`." That is the entire specification. Three consequences:

| | |
|---|---|
| **Cost** | one pass over the string: `len + 1` bytes read, every call. It is not a stored field. |
| **Precondition** | `s` is a string. If there is no terminator in the object, the behaviour is undefined — no diagnostic, no return value to check. |
| **Return** | `size_t`, the count. There is no error indication, because there is no error it could detect. |

The cost is why `for (i = 0; i < strlen(s); i++)` reads the string `n + 1` times for an
`n`-character string. The compiler may hoist the call out of the loop when it can prove the
string is unchanged, and at `-O0`, or whenever the body calls anything, it cannot. Measure
once into a variable, or write the loop that stops at the terminator and needs no length at
all.

The precondition is why this unit has two `[BUG]` exercises and a `[REPORT]`: a missing
terminator is never detected where it was created, only where somebody later asks for a
length.

### `size_t` and the wrap at zero

`size_t` is unsigned, and unsigned arithmetic in C does not overflow: a result that cannot be
represented is reduced modulo one plus the largest representable value (C17 6.2.5p9). So:

```
size_t arithmetic at zero

  SIZE_MAX                  = 18446744073709551615
  strlen("")                = 0
  strlen("") - 1            = 18446744073709551615
  (size_t)0 - 1 == SIZE_MAX  : yes
  0 <= strlen("") - 1        : yes   (the loop that never ends)
```

`strlen("") - 1` is `SIZE_MAX`, and `i <= SIZE_MAX` is true for every `size_t`. A loop with
that bound has no exit — not a long loop, no exit — and no comparison can rescue it, because
there is no `size_t` greater than `SIZE_MAX`.

The rule that removes the whole class: **never subtract from a length you have not proved is
non-zero.** In practice that means preferring these shapes:

| Instead of | Write | Why |
|---|---|---|
| `i <= strlen(s) - 1` | `s[i] != '\0'` | no length, no subtraction |
| `i <= n - 1` | `i < n` | correct for `n == 0`; the half-open range `[0, n)` |
| `buf[strlen(buf) - 1]` | test the length first, then index | the empty string would write `buf[-1]` |
| `for (i = n - 1; i >= 0; i--)` | `for (i = n; i-- > 0; )` | an unsigned `i` is always `>= 0` |

The third line is S12 e08 and the first is e03 of this unit.

### A length that does not fit its type

A length is a `size_t`. The moment it is stored in anything narrower — a one-byte length
field, a `uint16_t` record header, an `int` return value — the value has to fit, and C will
not tell you when it does not. For unsigned types the conversion is *defined*: the value is
reduced modulo 2^N (C17 6.3.1.3p2). From the same demo:

```
a length that does not fit its type

  300 stored in unsigned char, read back  = 44
  256 stored in unsigned char, read back  = 0
  255 stored in unsigned char, read back  = 255
```

300 became 44 and nothing is wrong, as far as the language is concerned: that is what the
conversion means. No sanitizer reports it (nothing is out of bounds, nothing is undefined),
and the compiler is silent unless you turn on `-Wconversion`, which nobody can leave on. What
is left is a *check*, at the conversion, against what the destination can represent —
`UCHAR_MAX`, or better the format's own documented maximum. e06 is a record that forgot it.

### Fixed-width fields: the full field

A fixed-width field is `width` bytes of text, padded with NUL bytes when the text is shorter.
It is the oldest record layout in Unix and still the shape of protocol headers, database rows
and `tar` archives, because it makes a record's fields findable by arithmetic rather than by
parsing.

The case that matters is the **full** field: text exactly `width` characters long, with no room
for a terminator. It is not an edge case, it is what a format whose widths were chosen to fit
the data produces constantly, and it means:

| Question | Unbounded, needs a terminator | Bounded, does not |
|---|---|---|
| how long is the text? | `strlen(field)` | `strnlen(field, width)` |
| where is this byte? | `strchr(field, c)` | `memchr(field, c, width)` |
| copy it out | `strcpy(dst, field)` | `memcpy(dst, field, n)` and terminate |
| print it | `printf("%s", field)` | `printf("%.*s", (int)width, field)` |
| compare two | `strcmp(a, b)` | lengths, then `memcmp(a, b, len)` |

The right-hand column never needs the convention, and every entry in it takes the bound from
the *format*, which is a number the program already knows. e02 builds the first, third and
fifth of those rows.

One more property of a padded field, and the reason `strncpy` was invented (S03): the padding
makes equal text into equal bytes, so records can be compared and sorted without parsing. The
reader still has to know that the padding is not part of the value — comparing all `width`
bytes reports two records with the same name as different the moment one writer did not zero
its unused bytes.

## Function reference

### `strlen`

```c
#include <string.h>
size_t strlen(const char *s);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.6.3). The Linux page's STANDARDS line reads `C11, POSIX.1-2008`. |
| **Does** | Counts the bytes of the string `s` before its terminator. Reads forward from `s[0]` and stops at the first zero byte. |
| **Returns** | The count, as `size_t`. 0 for the empty string. Never an error indication. |
| **Requires** | `s` is a string: there is a zero byte at or after `s[0]`, and every byte up to and including it is readable. |
| **Undefined if** | There is no terminator within the object; `s` is a null pointer. |
| **Reads / writes** | Reads `strlen(s) + 1` bytes. Writes nothing. One pass. |
| **Terminated afterwards?** | none: it changes nothing. |
| **Reports truncation?** | none. |
| **Why it is shaped this way** | It is the oldest function in the library and dates from a time when a string *was* a pointer; a bound would have had nowhere to come from. The `const` and the `size_t` were added by C89, which is why the shape looks modern and the contract does not. |
| **The usual mistake** | Calling it on a fixed-width field that may be full, on a buffer a bounded copy may have left unterminated (`strncpy`, S03), or on a heap block that was allocated for the text and not the terminator (e05). Second: putting it in a loop condition, which makes the loop quadratic. |
| **Use instead** | `strnlen(s, size)` when the bytes may not be a string; `memchr(s, '\0', size)` for the same in strict ISO C; a stored length when you control the data structure. |
| **Use it for** | Any string whose terminator you put there, or that came from a function documented to produce one (`strdup`, `snprintf`, `getline`). That is most strings, which is why the function is not a mistake — only its careless uses are. |

```
s:      h   e   l   l   o  \0            strlen(s) = 5      6 bytes read
s:      \0                               strlen(s) = 0      1 byte read
s:      a   b  \0   c   d  \0            strlen(s) = 2      3 bytes read; the cd is unreachable
s:      A   1   2   3 | ? ...            undefined: it reads past the object looking for a \0
```

### `strnlen`

```c
#include <string.h>
size_t strnlen(const char *s, size_t maxlen);
```

| | |
|---|---|
| **Standard** | POSIX.1-2008. **Not in ISO C.** glibc declares it when `_POSIX_C_SOURCE >= 200809L`, which `lib/dsalloc.h` defines for every file in this course. |
| **Does** | Counts the bytes before the first zero byte among the first `maxlen` bytes at `s`. Stops at the first zero byte or at `maxlen`, whichever comes first. |
| **Returns** | The count, as `size_t`; `maxlen` if there is no zero byte among those bytes. So `ret == maxlen` means "no terminator was found" and `ret < maxlen` is a true length. |
| **Requires** | The first `min(maxlen, length + 1)` bytes at `s` are readable. `s` need **not** be terminated. `maxlen` may be 0, and then no byte is read. |
| **Undefined if** | Fewer than that many bytes are readable; `s` is a null pointer and `maxlen > 0`. |
| **Reads / writes** | Reads at most `maxlen` bytes. Writes nothing. One pass. |
| **Terminated afterwards?** | none. |
| **Reports truncation?** | It reports the thing truncation is usually detected by: `ret == maxlen` means the text filled the bound. |
| **Why it is shaped this way** | It was added for exactly the data `strlen` cannot handle: fixed-width fields and buffers from outside the program. Returning `maxlen` rather than an error keeps every result a usable number, which is why `memcpy(dst, f, strnlen(f, w))` needs no branch. |
| **The usual mistake** | Implementing or using it as "measure, then clamp": `min(strlen(s), maxlen)` returns the same numbers and reads past the end of a full field, which is the one thing the function exists to prevent. |
| **Use instead** | `memchr(s, '\0', maxlen)` where strict ISO C portability is needed; the identity is in the Linux manual page. |
| **Use it for** | Every measurement of bytes you did not terminate yourself: record fields, protocol payloads, `read()` buffers, anything a caller handed you with a size. |

```
s:      h   e   l   l   o  \0        strnlen(s, 6) = 5    the terminator is inside the bound
s:      h   e   l   l   o  \0        strnlen(s, 3) = 3    the bound came first
s:      h   e   l   l   o  \0        strnlen(s, 0) = 0    no byte is read at all
s:      A   1   2   3                strnlen(s, 4) = 4    full field: the bound, and s[4] is not read
```

## Worked example: measuring a field you did not write

A record format: an 8-byte name field, NUL-padded. You have the field and the width, and you
want the name as a string in a buffer of your own.

```c
size_t len = strnlen(field, 8);        /* 0 to 8; 8 means "full, no terminator" */
if (len >= cap)                        /* the string needs len + 1 bytes */
    return -1;
memcpy(dst, field, len);
dst[len] = '\0';
```

With `field` holding `ada\0\0\0\0\0` and `cap` 8: `strnlen` reads `a`, `d`, `a` and the zero
byte, stops, and returns 3. `3 >= 8` is false. `memcpy` moves three bytes, and `dst[3]` gets
the terminator, so `dst` is `ada\0####` — four bytes written, four untouched.

With `field` holding `maximili` (full) and `cap` 8: `strnlen` reads all eight bytes, finds no
zero byte, stops at the bound and returns 8. `8 >= 8` is true, so the function refuses: eight
characters and a terminator need nine bytes. Note what did *not* happen — nothing read
`field[8]`, and nothing was written to `dst`.

Three details carry the weight. The bound passed to `strnlen` is the **field's width**, which
comes from the format, not from the data. The test is `len >= cap`, not `>`, because the
terminator needs a byte. And `memcpy` is given `len`, the measured length, never `width`:
copying `width` bytes would bring the padding along, which for a padded field means writing
zero bytes you then overwrite, and for a *mis*padded field means copying junk.

**The plausible wrong reading.** "`strnlen(field, 8)` returning 8 means the name is eight
characters long, so I need nine bytes and otherwise everything is the same." The number is
right and the inference is not quite: 8 means *no terminator was found in eight bytes*, which
for this format means the name is exactly eight characters, and for a format whose writer was
sloppy might mean the field contains no name at all but eight bytes of something else. A
bounded measurement answers a question about the bytes you were given, not about the data the
writer intended. When the difference matters — and for a length byte, a checksum or an offset
it always matters — the format needs a validation step, not a smarter measurement.

## Worked example: the loop bound that never ends

```c
for (size_t i = 0; i <= strlen(s) - 1; i++)
    if (s[i] >= 'A' && s[i] <= 'Z')
        n++;
```

For `s = "Hello"`: `strlen(s)` is 5, the bound is 4, and `i` takes 0, 1, 2, 3, 4 — five
iterations, which is right. The loop also calls `strlen` five times, so it reads the string
six times in all, but it produces the right answer.

For `s = ""`: `strlen(s)` is 0, and `0 - 1` in `size_t` is `SIZE_MAX`:
18446744073709551615. The condition `i <= SIZE_MAX` is true for every value `i` can hold, so
the loop reads `s[0]` (the terminator — inside the object, and fine), then `s[1]`, which is the
first byte it does not own, and keeps going until the kernel stops the process. The empty
string is not an unusual input; it is the first line of an empty file.

The repair is not a special case. It is the loop that does not ask for a length:

```c
for (size_t i = 0; s[i] != '\0'; i++)
```

which is correct for the empty string without mentioning it, reads each byte once, and has no
subtraction to get wrong.

**The plausible wrong reading.** "`i <= strlen(s) - 1` and `i < strlen(s)` are the same
condition, so this is a style question." They are the same for every non-empty string and they
are not the same expression: one subtracts from an unsigned value and the other does not.
`i < n` is correct for `n == 0` by construction, which is why half-open ranges are the
convention everywhere in C — in the standard library's `(pointer, count)` parameters, and in
the `[32, 36)` of an AddressSanitizer report. A rule that costs nothing and removes a class of
bug is not a style question.

## What the tools say

**The compiler, when it can see the object.** `lesson-demo/warn.c` has three defects of this
unit. gcc catches one of them, from `lesson-demo/capture-warn.txt` (`-O2 -Wall -Wextra
-Wpedantic`; gcc prints the same diagnostic twice, from two different optimisation passes):

```
warn.c: In function ‘id_length’:
warn.c:13:19: warning: ‘strlen’ argument missing terminating nul [-Wstringop-overread]
   13 |     return strlen(id);
      |                   ^~
warn.c:8:19: note: referenced argument declared here
    8 | static const char id[4] = {'A', '1', '2', '3'};
      |                   ^~
warn.c:13:19: warning: ‘strlen’ argument missing terminating nul [-Wstringop-overread]
   13 |     return strlen(id);
      |                   ^~
warn.c:8:19: note: referenced argument declared here
    8 | static const char id[4] = {'A', '1', '2', '3'};
      |                   ^~
```

That is the best diagnostic in this unit: the argument of `strlen` is an object the compiler
can see, and it can see there is no zero byte in it. The `note:` line points at the
declaration, which is where the fix goes.

The same file through clang, from `lesson-demo/capture-warn-clang.txt`:

```
(no output: clang reports nothing about any of the three)
```

Nothing at all. And gcc said nothing about the other two defects either: not about the
`i <= strlen(s) - 1` loop, whose bound it computes at compile time, and not about
`return strlen(s);` from a function returning `unsigned char`, which is a narrowing conversion
`-Wall -Wextra` does not report. Three defects, one warning, and the one that was caught
needed the object to be visible in the same translation unit. **A warning is evidence of a
defect; no warning is evidence of nothing.**

**AddressSanitizer, for the full field.** `make asan ARGS=field` in `lesson-demo` calls
`strlen` on a four-byte global with no terminator; from `capture-field.txt` (paths and build
ids shortened):

```
==1761516==ERROR: AddressSanitizer: global-buffer-overflow on address 0x5e92f12f4a64 at pc 0x70c15ca8841c bp 0x7ffe11009470 sp 0x7ffe11008c28
READ of size 5 at 0x5e92f12f4a64 thread T0
    #0 0x70c15ca8841b  (/usr/lib/libasan.so.8+0x8841b)
    #1 0x5e92f12eb4fb in len_of .../lesson-demo/main.c:18

0x5e92f12f4a64 is located 0 bytes after global variable 'field4' defined in 'main.c:45:23' (0x5e92f12f4a60) of size 4
```

A third shape to add to the two in S00. `global-buffer-overflow` means the address is just
outside a variable with static storage duration, and the "is located" line names it — the
variable, the file and line it is **defined** on, and its size. For a global there is no
allocation stack to read and none is needed: the declaration is the report.

**AddressSanitizer, for the loop that wraps.** `make asan ARGS=wrap` runs the loop of the
second worked example on the empty string; from `capture-wrap.txt`:

```
==1761496==ERROR: AddressSanitizer: global-buffer-overflow on address 0x586cde83c161 at pc 0x586cde833944 bp 0x7ffcaf7cbfa0 sp 0x7ffcaf7cbf90
READ of size 1 at 0x586cde83c161 thread T0
    #0 0x586cde833943 in main .../lesson-demo/main.c:57
    #1 0x7590bc627780  (/usr/lib/libc.so.6+0x27780)

0x586cde83c161 is located 0 bytes after global variable '*.LC6' defined in 'main.c' (0x586cde83c160) of size 1
  '*.LC6' is ascii string ''
0x586cde83c161 is located 63 bytes before global variable '*.LC7' defined in 'main.c' (0x586cde83c1a0) of size 12
  '*.LC7' is ascii string 'upper: %zu
'
```

The empty string here is a literal, so the object the loop walks off is again a global — and
ASan names it with the compiler's internal label (`*.LC6`) and tells you what it holds:
`is ascii string ''`. A one-byte region whose contents are the empty string, overrun by one
byte, with the loop in frame `#0`: the report describes the defect exactly. Compare it with
the plain build, where nothing stops the loop until it leaves the heap and `make` prints only
`Error 139` — 128 + 11, killed by `SIGSEGV`.

**Nothing at all.** The narrowing conversion of e06 produces no report from any tool, on any
platform, in any build: 301 bytes are copied into a 512-byte buffer and a 300 is stored in one
byte as a 44. Only a check on the value catches it. That is the one defect class in this
course for which the tools are no help at all, and it is why `[BUG]` exercises exist alongside
`[REPORT]` ones.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| length vs. size | Length counts the bytes before the terminator. Size is the whole buffer. A string fits when `len < cap`. |
| `strlen` vs. `strnlen` | `strlen` requires a terminator and takes no bound; `strnlen` requires nothing and takes a bound. Only the second may be pointed at a field that might be full. |
| `strnlen` returning a length vs. returning the bound | `ret < maxlen` is a true length. `ret == maxlen` means no terminator was found, and says nothing about how long the text "really" is. |
| a bounded measurement vs. a clamped one | `strnlen(s, n)` reads at most `n` bytes. `min(strlen(s), n)` gives the same number and reads the whole string. |
| `strlen(s) - 1` vs. `strlen(s)` | The first is `SIZE_MAX` for the empty string. Any subtraction from an unsigned length needs the zero case ruled out first. |
| `i <= n - 1` vs. `i < n` | The same loop for `n > 0`; the second is also correct for `n == 0`. |
| a length in `size_t` vs. in a format field | A conversion sits between them, it is defined, it loses information silently, and only a range check catches it. |
| padding vs. value | In a NUL-padded field the bytes after the first NUL are not part of the text. Comparing all `width` bytes compares them anyway. |
| the length of the text vs. the bytes allocated | A heap string needs `len + 1` bytes. A block of `len` bytes holding `len` characters is the defect of e05. |

## `man` pointers

- `man 3 strlen`: SYNOPSIS for the `const` and the `size_t`, DESCRIPTION for "excluding the
  terminating null byte", STANDARDS for `C11, POSIX.1-2008`.
- `man 3 strnlen` (Linux; on macOS it shares the `strlen(3)` page): DESCRIPTION, which calls
  its argument a *character sequence* rather than a string, and gives the identity
  `(memchr(strn, '\0', n) ?: strn + n) - strn`. Note the **Feature Test Macro Requirements**
  block between the SYNOPSIS and the DESCRIPTION: `_POSIX_C_SOURCE >= 200809L`.
- `man 7 feature_test_macros` (Linux only): why that block exists and what the macros do.
- `man 7 string_copying` (Linux only), **Terms**: string, character sequence, null-padded
  character sequence, length, size, end. The four words of this unit, defined by the people
  who wrote the pages.
- `man 3 memchr` (both): the ISO C way to do what `strnlen` does, for code that may not use
  POSIX.

## Check yourself

1. `char f[8] = "hi";` What are `sizeof f`, `strlen(f)` and `strnlen(f, 8)`, and which of the
   three is the capacity to pass to a bounded copy?
2. `strnlen(s, 4)` returns 4. What do you know about `s`, and what do you *not* know?
3. Write the condition that is true exactly when the string `s` fits in `cap` bytes, using a
   function that is safe even if `s` has no terminator.
4. What is `strlen("") - 1`, and what is its type?
5. A 12-byte field holds `"report\0\0xxxx"`. What does `strnlen(field, 12)` return? How many
   bytes should a copy of the text take?
6. Why does `for (size_t i = 0; i < strlen(s); i++)` read an `n`-character string more than
   once, and what are the two ways to fix it?
7. A record stores a length in a `uint16_t`. Which check does the code need, where, and what
   do the sanitizers say if it is missing?
8. An ASan report says `READ of size 9` and `0 bytes after 8-byte region`, with `strlen` on
   top. What happened, and which line is wrong?

<details><summary>Answers</summary>

1. 8, 2, 2. The capacity is `sizeof f` (8): a bounded copy takes the buffer's size and works
   out the `cap - 1` itself.
2. You know there is no zero byte in the first four bytes at `s`, so those four bytes are not a
   string. You do not know how long the text "really" is, whether there is a terminator at
   `s[4]`, or whether reading `s[4]` is even allowed.
3. `strnlen(s, cap) < cap`.
4. `SIZE_MAX` (18446744073709551615 on both course platforms), of type `size_t`. Unsigned
   subtraction wraps; it does not go negative.
5. 6. The text is `report`, six characters, so a copy as a string needs seven bytes — and the
   `xxxx` after the padding is not part of it.
6. `strlen(s)` is re-evaluated on each iteration unless the compiler can prove it is
   unchanged. Fix it by measuring once into a `size_t` before the loop, or by writing
   `s[i] != '\0'` and not measuring at all.
7. A test that the length is at most `UINT16_MAX` (or the format's documented maximum),
   immediately before the conversion that stores it. The sanitizers say nothing: the
   conversion is defined and nothing goes out of bounds.
8. Something asked for the length of an 8-byte block that holds eight characters and no
   terminator; `strlen` read the eight plus one more and was stopped. The wrong line is
   neither the `strlen` nor the block's reader: it is the allocation or the copy that produced
   eight bytes where nine were needed.

</details>

## Takeaways

- `strlen` is a measurement with a cost and a precondition: one pass over the data, and a
  terminator that must already be there.
- A length and a size are different numbers; a string of length `len` fits in `cap` bytes when
  `len < cap`, and `cap` always includes the terminator.
- `strnlen(s, n)` is the measurement for bytes you did not terminate yourself, and `ret == n`
  is how it says "there is no terminator in here".
- A bounded function that measures first and clamps afterwards is not bounded.
- `size_t` is unsigned, so `length - 1` on an empty string is `SIZE_MAX`; prefer `i < n` and
  loops that stop at the terminator.
- Storing a length in a narrower type is a defined, silent, lossy conversion, and only a range
  check at the conversion catches it.
- A heap `READ` of one byte more than the region, with a string function on top, means a block
  sized for the text and not for the terminator.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-my-strlen/` | `[WRITE]` | S | `my_strlen` and `my_strnlen`, oracle-tested against libc |
| e02 | `e02-fixed-fields/` | `[WRITE]` | S | measure, compare and copy out NUL-padded fixed-width fields |
| e03 | `e03-empty-wraps/` | `[BUG]` | | `i <= strlen(s) - 1` on the empty string |
| e04 | `e04-length-predict/` | `[PREDICT]` | | sixteen measurements of five objects |
| e05 | `e05-unterminated-report/` | `[REPORT]` | | a heap over-read read back to a `malloc` that was one byte short |
| e06 | `e06-narrow-length/` | `[BUG]` | | a 300-character name in a one-byte length field, and no tool says a word |
| e07 | `e07-strlen-lookup/` | `[LOOKUP]` | | nine facts about `strlen` and `strnlen`, with quotes |

## Drills

| # | Folder | Skill | Target |
|---|---|---|---|
| r01 | `drills/r01-strlen-reps/` | `strlen` three ways, plus the `len < cap` test | 8 min |
| r02 | `drills/r02-end-pointer-reps/` | end pointers: where the string ends, how much room is left | 8 min |
| r03 | `drills/r03-count-reps/` | four one-pass counts: bytes, words, lines, the longest run | 10 min |
