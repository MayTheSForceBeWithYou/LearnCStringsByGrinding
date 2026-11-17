# S00: Bytes, arrays and the terminator

## Organizer

C has no string type. It has a **convention**: some bytes, followed by a zero byte, and a
pointer to the first of them. Every function in `<string.h>` is built on it, every one trusts
it without checking, and most of the defects in the rest of this course are a program that
broke it in one place and found out in another.

This unit is about the convention itself, before any library function. What changes in your
thinking: you stop reading `char buf[16]` as "a string of up to 16 characters" and start
reading it as "sixteen bytes, of which the text may use fifteen, if somebody writes the zero".
Three questions become automatic for any declaration — **how many bytes does it reserve, how
many are in use, and may I write them** — answered by three different things.

You need the memory course's M01 and M02 (objects, the stack, the heap, what a pointer is).
Nothing from S01 onwards.

After this unit you can:

- say, for any `char` declaration, how many bytes it reserves, how many characters it holds,
  whether it is a string at all, and whether its bytes may be written;
- state the difference between `sizeof` and `strlen`: which is a fact about a type, which is a
  measurement of data;
- count the bytes of a string literal, including the ones you did not write, and say where
  they live, and predict any expression over a byte above 0x7F in `char`, `signed char` or
  `unsigned char`;
- read a `stack-buffer-overflow` report back to the declaration that was one byte too small;
- write a string's length, end, character count and a bounded fill from a blank file, with no
  library call.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| `sizeof` | — (an operator) | ISO C | below | the bytes an object or type occupies, decided at compile time |
| `puts` | `<stdio.h>` | ISO C | below | write a string and a newline to standard output |
| `strlen` | `<string.h>` | ISO C | S01 | count the bytes before the terminator; a black box until S01 |
| `printf` with `"%s"` | `<stdio.h>` | ISO C | S11 | write the bytes of a string until its terminator |

`sizeof` is not a function, and `strlen` and `%s` appear here only as consumers of the
convention, to show what they assume; their cards are in the units that take them apart.

## Foundations

### A string is four words

The Linux manual page `string_copying(7)` opens with the vocabulary, worth adopting word for
word because the library uses it and so does the rest of this course:

| Word | Means |
|---|---|
| **string** | zero or more non-zero bytes followed by one zero byte |
| **character sequence** | zero or more non-zero bytes, with no terminator |
| **length** | the number of non-zero bytes: what `strlen` returns |
| **size** | the whole buffer the bytes sit in: what `sizeof` or a `cap` parameter says |

A string is a character sequence *plus a promise*, and the promise is one byte. A buffer has
a size; the text in it has a length; and a string of length `len` occupies `len + 1` bytes, so
it fits in a buffer of size `cap` exactly when `len < cap`. Write the test that way every
time: `len <= cap` is the off-by-one that forgets the terminator. In this course a parameter
named `cap` or `size` always *includes* the terminator and one named `len` or `n` never does.

### Where the bytes live

Four declarations — `char a[] = "abc"`, `const char *p = "abc"`,
`const char *v[3] = {"a", "bb", "ccc"}` and `char m[3][8] = {"ab", "cd", "ef"}` — and four
different arrangements of storage behind text that looks alike:

```
a:    a   b   c  \0           in the frame (or the data segment), writable

p:   [ address ]  ──→   a   b   c  \0        p is 8 bytes; the literal is read-only

v:   [ addr ][ addr ][ addr ]                v is 24 bytes
        │       │       │
        ↓       ↓       ↓
      a \0    b b \0   c c c \0              three literals, read-only

m:    a   b  \0  \0  \0  \0  \0  \0          m is 24 bytes in one block:
      c   d  \0  \0  \0  \0  \0  \0          m[1] starts 8 bytes after m[0],
      e   f  \0  \0  \0  \0  \0  \0          and every byte not initialised is zero
```

| | Who owns the bytes | Writable | `sizeof` is | Text can grow |
|---|---|---|---|---|
| `char a[] = "abc"` | you | yes | the array: 4 | no: the size is fixed at compile time |
| `const char *p = "abc"` | the compiler | **no** | the pointer: 8 | no, but `p` can point elsewhere |
| `const char *v[3]` | you own the pointers | the pointers, yes; the literals, no | the array of pointers: 24 | each entry can point at a longer string |
| `char m[3][8]` | you | yes | the whole block: 24 | no: 8 bytes per row, always |

Two rules come out of this table, both worth memorising. **A string literal is the compiler's
array, not yours**: point at it with `const char *` and copy it into your own buffer before
changing a byte (e03 is a program that forgot). And **`sizeof` is a fact about the type**, so
it is 8 for `p` whatever `p` points at — which is why `sizeof` serves as a capacity only where
the array itself is in scope, never after it has been passed to a function.

### What a string literal is

`"abc"` is an array of four `char`: the three characters and a terminator the compiler
appends. It has static storage duration, so it exists for the whole run, and the standard
says that if the program tries to modify it the behaviour is **undefined** (C17 6.4.5p7).

Its type in C is `char[4]`, not `const char[4]` — C never made literals `const`, to avoid
breaking code written before `const` existed — so `char *q = "abc";` compiles without a murmur
and the write through `q` is still undefined. The type system does not protect you here; the
convention `const char *` does.

Four more properties, each of which shows up in e01:

| Written | Is | Bytes |
|---|---|---|
| `"ab" "cd"` | one literal: adjacent literals are joined in translation phase 6 | `a b c d \0` |
| `"a\0b"` | one literal with an embedded zero byte, four bytes long | `a \0 b \0` |
| `""` | the empty string: one byte, and that byte is the terminator | `\0` |
| `char c[8] = "abc"` | an 8-byte array; the rest is zero-initialised (C17 6.7.9p21) | `a b c \0 \0 \0 \0 \0` |

And `char code[3] = "abc";` is legal, means exactly what it says — three characters, no
terminator — and is almost always a mistake: the initialiser may drop the terminator when the
array is exactly as long as the characters (C17 6.7.9p14), so the result is a character
sequence that `strlen` must not be given. Both course compilers now warn about it; see
**What the tools say**.

Inside a literal a backslash starts an escape: `\0` is the zero byte (it is `\ooo` octal with
one digit), `\n` `\t` `\r` are 10, 9 and 13, `\\` is one backslash, `\"` is a quote, `\ooo`
takes at most three octal digits, and `\xHH` takes **as many hex digits as follow**, which is
the trap. `\x` is greedy: it consumes every hex digit that follows, not two. `"\x41"` is one byte, `A`,
but `"\x411"` is not `A1`; it is the single value 0x411, which does not fit in a `char`, and
the program is ill-formed. Split the literal when the text continues with a hex digit:
`"\x41" "1"`. Octal is the opposite — `"\0101"` is two bytes, because `\010` stops after three
digits. This is also why the course's byte notation always writes `\xHH` with exactly two
upper-case digits (drill r03 builds that renderer): a rendering has to be readable back.

### `sizeof` against `strlen`

Here is the whole unit in one table, printed by `lesson-demo` (`make run` there):

```
what a declaration really reserves

  declaration                    sizeof   strlen   the bytes
  -----------                    ------   ------   ---------
  char a[] = "abc";                   4        3   abc\0
  const char *p = "abc";              8        3   abc\0
  char b[3] = {'a','b','c'};          3        -   abc
  char c[8] = "abc";                  8        3   abc\0\0\0\0\0
  char d[] = "ab" "cd";               5        4   abcd\0
  char e[] = "a\0b";                  4        1   a\0b\0
  char f[] = "";                      1        0   \0
  char r[] = "caf\xC3\xA9";           6        5   caf\xC3\xA9\0
  char m[3][8] = {"ab",...};         24        2   ab\0\0\0\0\0\0cd\0\0\0\0\0\0ef\0\0\0\0\0\0

  For p, sizeof is the pointer and the bytes shown are the literal's.
  For b, strlen has no answer: there is no terminator in the three bytes.
```

Read down the `sizeof` column: storage, and for `p` the pointer. Down the `strlen` column:
text, and for `b` no answer at all. Then the bytes, where the surprises live — `c` has five
bytes nobody wrote, `e` has a byte after its terminator, and `m` is one block of 24 whose
every row is padded to 8.


### One byte, three types

A byte of text above 0x7F is where the character types stop agreeing. `char` is one byte wide
and the standard deliberately does not say whether it is signed (C17 6.2.5p15); it is signed
on x86-64 Linux and on macOS, unsigned on ARM Linux. And a `char` almost never stays a `char`:
the **integer promotions** (C17 6.3.1.1p2) convert it to `int` before any comparison,
arithmetic, array subscript or variadic call.

So the question about a byte is never "what is in it" but "what `int` does it become". The
same `lesson-demo` prints:

```
the byte 0xE9, promoted to int

  declaration      as int       > 0      < 'a'      >> 4
  -----------      ------       ---      -----      ----
  signed char sc      -23     false       true        -2
  unsigned char uc    233      true      false        14
  char pc             -23     false       true        -2
```

Three consequences, all of which are real bug classes:

- **The sign test.** `c > 0` is a test for "an ASCII character" only if `c` is unsigned.
- **Ordering.** A hand-written comparison on `signed char` sorts every byte above 0x7F
  *before* `'a'`, while `strcmp`, which is specified to compare as `unsigned char`, sorts them
  after. S06 e03 is a binary search that misses keys for this reason.
- **Subscripts.** A 256-entry class table indexed by a plain `char` reads *before* the table
  for every byte above 0x7F: `letter[c]` with `c` holding 0xE9 is `letter[-23]`, undefined in
  the arithmetic and not merely in the dereference (C17 6.5.6p8), and in practice an answer
  read from whatever precedes the table. That is `isalpha(c)` with a plain `char`, exactly:
  S07 e02, and e05 works out every value involved.

The rule, for the rest of the course: **a byte of data lives in an `unsigned char`**, and
where it must become an `int`, cast it: `table[(unsigned char)c]`,
`isalpha((unsigned char)c)`, `printf("%02X", (unsigned char)c)`. Plain `char` means "a
character of text" and its signedness is the platform's business.

### What "terminated" means, and who checks

Nothing in the language or the library records whether a buffer holds a string. `strlen`,
`strcpy`, `strcmp`, `printf("%s")` and the rest read until they find a zero byte, because that
is the only stopping condition they have. The standard phrases this as a requirement on *you*:
the parameter is documented as a string, and C17 4p2 makes the behaviour undefined when a
program breaks that requirement — no diagnostic, no return value, nothing in the interface
where the failure could be reported.

So "is this a string?" must be answered by the code that produced the bytes; and where it
cannot be, by a bounded function instead of an unbounded one:

| You have | Unbounded (needs the convention) | Bounded (does not) |
|---|---|---|
| a measurement | `strlen(s)` | `strnlen(buf, cap)` (S01) |
| a search | `strchr(s, c)` | `memchr(buf, c, cap)` (S02) |
| printing | `printf("%s", s)` | `printf("%.*s", (int)n, buf)` (S11) |
| a copy | `strcpy(dst, src)` | `memcpy(dst, src, n)` (S02) |

That right-hand column is why the `mem` functions exist and why S02 comes before everything
else that copies. A field of exactly its own width — four characters in four bytes — is not a
defect; treating it as a string is. e02 asks you to write `buf_is_terminated(buf, cap)` by
hand, because it is the question the library cannot ask for you.

## Function reference

### `sizeof`

```c
sizeof expression        /* no parentheses needed for an expression */
sizeof (type-name)       /* parentheses required for a type */
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 6.5.3.4). An operator, not a function; there is no header. |
| **Does** | Yields the number of bytes in the representation of its operand's **type**. The operand is not evaluated (except for a variable-length array, which this course does not use), so `sizeof arr[i++]` does not increment `i`. |
| **Returns** | A value of type `size_t`. `sizeof (char)` is 1 by definition, so for a `char` array the answer is its element count. |
| **Requires** | A complete type. Not applicable to a function type, an incomplete type, or a bit-field. |
| **Undefined if** | none — it cannot fail, and it reads no memory. |
| **Reads / writes** | Neither. It is computed at compile time and the program contains the constant. |
| **Terminated afterwards?** | none: it changes nothing. |
| **Reports truncation?** | none. |
| **Why it is shaped this way** | It is the only way for portable code to know how large a type is, and it was deliberately made a compile-time constant so that it can size arrays and appear in constant expressions. |
| **The usual mistake** | `sizeof p` where `p` is a pointer, used as the capacity of what `p` points at: always 8 here, whatever the buffer. A close second: `sizeof arr` inside a function that received `arr` as a parameter, where the array type has already decayed to a pointer. |
| **Use instead** | Where the array is not in scope, a `cap` parameter the caller filled in with `sizeof` at the declaration. There is no way to recover a size from a pointer. |
| **Use it for** | `char buf[64]; ... snprintf(buf, sizeof buf, ...)`: in the declaration's own scope `sizeof` is exactly right, and repeating the 64 by hand is the defect. Also `malloc(n * sizeof *p)`, which cannot go out of step with `p`'s type. |

```
char a[] = "abc";          sizeof a      = 4     the array
const char *p = "abc";     sizeof p      = 8     the pointer
                           sizeof *p     = 1     one char
char m[3][8];              sizeof m      = 24    the whole block
                           sizeof m[0]   = 8     one row (8 again, for another reason)
                           sizeof "abc"  = 4     the literal, terminator included
```

### `puts`

```c
#include <stdio.h>
int puts(const char *s);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.21.7.9) |
| **Does** | Writes the bytes of the string `s` to standard output, then a newline. The terminator is not written. |
| **Returns** | A nonnegative number on success, `EOF` on an error. It does **not** promise to return the number of characters written. |
| **Requires** | `s` is a string. The stream is writable. |
| **Undefined if** | `s` has no terminator, or is a null pointer. (`printf("%s", NULL)` is undefined too; glibc happens to print `(null)`, which is not something to rely on — SPEC section 10.) |
| **Reads / writes** | Reads `strlen(s) + 1` bytes; writes `strlen(s) + 1` bytes to the stream, of which the last is a newline. |
| **Terminated afterwards?** | none: it writes to a stream, not to a buffer. |
| **Reports truncation?** | No, but it does report failure: a full disk or a closed pipe gives `EOF`. |
| **Why it is shaped this way** | It is the one-line case of "print this text and end the line", and it takes no format string, so it cannot be given one by accident — the defect of S11 e03. |
| **The usual mistake** | Testing `puts(s) == 0`, which the standard never promised; and reaching for `printf(s)` when `puts(s)` was wanted, which turns user text into a format string. |
| **Use instead** | `fputs(s, stdout)` when you do not want the newline; `printf` when there is something to format. |
| **Use it for** | Fixed messages and already-formatted lines. `puts(buf)` after building `buf` is the common shape, and it is the cheapest way to print a line. |

```
puts("abc")      writes:  a   b   c   \n          returns a nonnegative number
puts("")         writes:  \n                      the string is empty; the newline is not
```

## Worked example: how many bytes does this declaration reserve?

Two declarations, and the question that has to be answered before any copying happens.

```c
char c[8] = "abc";
char m[3][8] = {"ab", "cd", "ef"};
```

For `c`: the size is stated, so it is 8 bytes. The initialiser supplies four of them (`a`,
`b`, `c` and the terminator the compiler appends), and C17 6.7.9p21 requires the rest to be
initialised as if they had static storage duration — for `char`, zero:

```
index:   0   1   2   3   4   5   6   7
c:       a   b   c  \0  \0  \0  \0  \0        sizeof c = 8, strlen(c) = 3
```

The useful number is neither 8 nor 3: it is `sizeof c - 1`, the longest string `c` can hold.
Four of those five zero bytes are room to grow; the fifth terminates the string that is there
now. For `m`, three rows of eight bytes in one block of 24, laid out end to end, each row
zero-filled to its width:

```
m[0]:    a   b  \0  \0  \0  \0  \0  \0
m[1]:    c   d  \0  \0  \0  \0  \0  \0        &m[1][0] - &m[0][0] = 8
m[2]:    e   f  \0  \0  \0  \0  \0  \0        sizeof m = 24, sizeof m[1] = 8
```

`m[1]` is an *array* of 8 bytes: not a pointer, not assignable, and `sizeof m[1]` is 8 because
the row is 8 bytes — a number that happens to equal a pointer's size on both course platforms,
so the two 8s in a printout mean different things. `m[1]` does *convert* to a pointer when
used as a value, which is why `strlen(m[1])` works and gives 2.

**The plausible wrong reading.** "`char m[3][8]` is an array of three strings, so it holds
three strings of up to eight characters." It holds three *character sequences* of up to eight
bytes, and they are strings only while each row's text is at most seven characters long. Write
exactly eight characters into a row — which `strncpy(m[1], src, 8)` will do without complaint
(S03) — and that row has no terminator, while the rows on either side of it are untouched and
still look fine. The fixed-width field is the oldest shape in Unix and the one that produces
the most unterminated buffers.

## Worked example: a field that is not a string

A record format says: four characters of identifier, a comma, a count — `"A123,7"`. The
obvious code copies the field out and measures it:

```c
char id[4];
field_copy(id, line, 4);          /* exactly four bytes, no terminator */
size_t n = strlen(id);            /* <-- */
```

The copy is correct: a four-character field in four bytes is what the format says. The defect
is on the last line, and it is not a defect in `strlen` either, which is specified to read
until it finds a zero byte — and there is no zero byte in those four:

```
index:   0   1   2   3  | 4   5   6 ...
id:      A   1   2   3  | ?   ?   ?        the bytes from index 4 on belong to something else
                        ^ strlen keeps reading here
```

What happens next is not predictable, and that is the point. If the next byte happens to be
zero, `strlen` returns 4 and the program works — the outcome you are most likely to get while
testing, which is why the defect ships. If it is not, the length is too large and every later
copy of "the id" takes rubbish with it. Under ASan the byte after `id` is a redzone, so the
read is reported with the variable's name and its declaration line: that is e04.

Three repairs, and they are not equivalent:

| Repair | Effect |
|---|---|
| `char id[5]` and write `id[4] = '\0'` after the copy | the field becomes a string; every later reader is safe |
| keep four bytes, measure with `strnlen(id, 4)` (S01) | no terminator needed; the bound travels with the bytes |
| keep four bytes, print with `printf("%.*s", 4, id)` (S11) | the same idea for output |

Real protocol and database code chooses the second: fields stay fixed width, and the width is
carried as a number. The first is right when the value escapes into code that expects strings.

**The plausible wrong reading.** "Add one to the buffer and the bug is gone, so the rule is
to make buffers one byte bigger than you think." The rule is to decide, for every buffer,
whether it holds a *string* or a *character sequence*, and then to use functions of the
matching kind. A program that makes every buffer one byte bigger and keeps reading
unterminated bytes with `strlen` has moved the overflow by one byte, not removed it: the first
record whose id is five characters is back where it started.

## What the tools say

**The compiler, when it can see the mistake.** From `lesson-demo/capture-warn.txt` (gcc at
`-O2 -Wall -Wextra -Wpedantic`):

```
warn.c:8:16: warning: initializer-string for array of ‘char’ truncates NUL terminator but destination lacks ‘nonstring’ attribute (4 chars into 3 available) [-Wunterminated-string-initialization]
    8 | char code[3] = "abc";
      |                ^~~~~
warn.c: In function ‘clear’:
warn.c:13:25: warning: argument to ‘sizeof’ in ‘memset’ call is the same expression as the destination; did you mean to provide an explicit length? [-Wsizeof-pointer-memaccess]
   13 |     memset(p, 0, sizeof p);
      |                         ^
```

clang warns about both of those in its own words, and about a third thing gcc passes over.
From `lesson-demo/capture-warn-clang.txt` (its first two warnings, which match gcc's, and two
notes omitted):

```
warn.c:19:20: warning: adding 'int' to a string does not append to the string [-Wstring-plus-int]
   19 |     return "hello" + 2;
      |            ~~~~~~~~^~~
```

Recognition rule: the flag in square brackets names the defect class, and the quoted
identifier or expression is what the compiler examined. Both compilers catch
`char code[3] = "abc"` — the trap is now mostly historical, which is why the exercises build
an unterminated array with braces instead — and both catch `sizeof` on a pointer handed
straight to `memset`. They differ on the third: clang objects to `"hello" + 2` under
`-Wstring-plus-int`, because adding an integer to a literal is more often a mistake than an
intention, and gcc says nothing. Build with both compilers when you can; each sees things the
other does not.

**AddressSanitizer, for the over-read.** `lesson-demo` performs the worked example's `strlen`
with `make asan ARGS=overread`; from `capture-asan.txt` (paths and build ids shortened):

```
==1609150==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7073074d00b4 at pc 0x74730a28841c bp 0x7ffc039627f0 sp 0x7ffc03961fa8
READ of size 5 at 0x7073074d00b4 thread T0
    #0 0x74730a28841b  (/usr/lib/libasan.so.8+0x8841b)
    #1 0x617643b9fcbe in main .../lesson-demo/main.c:61
    (frames #2 to #4 are inside libc and the C runtime)

Address 0x7073074d00b4 is located in stack of thread T0 at offset 180 in frame
    #0 0x617643b9f826 in main .../lesson-demo/main.c:46

  This frame has 9 object(s):
    (five other objects of this frame omitted)
    [176, 180) 'id' (line 58) <== Memory access at offset 180 overflows this variable
```

Read it in this order. **The first line** names the kind, `stack-buffer-overflow`, which says
which storage and therefore which table at the bottom of the report to read. **The second
line** is `READ`, and its size is how far the string function got: five bytes, the field plus
the first byte that was not the field. For strings a READ overflow almost always means a
missing terminator, and a WRITE overflow a copy that did not fit. **Frame `#0`** is inside the
sanitizer's own library, because ASan replaces `strlen` and its relatives with checking
versions (clang names that frame `in strlen`); **frame `#1`** is the first line of your code.
**The stack-object table** is the prize: it names the variable, its four bytes and its
declaration line, and marks the offset that overflowed it.

**AddressSanitizer, for the write to a literal.** With `make asan ARGS=literal`, from
`capture-segv.txt`:

```
AddressSanitizer:DEADLYSIGNAL
=================================================================
==1609167==ERROR: AddressSanitizer: SEGV on unknown address 0x579cd98e25c0 (pc 0x579cd98d9e0a bp 0x7ffda8e35fa0 sp 0x7ffda8e35e20 T0)
==1609167==The signal is caused by a WRITE memory access.
    #0 0x579cd98d9e0a in main .../lesson-demo/main.c:67
```

A different shape entirely: no "is located N bytes after", no allocation stack, no object
name. `SEGV on unknown address` means the address belongs to nothing ASan tracks — here a
read-only page of the executable — and the useful line is the one naming a `WRITE` access,
which rules out every missing-terminator defect. In the plain build the same program dies with
no output at all and `make` reports `Error 139`: 128 + 11, killed by signal 11, `SIGSEGV`.
e03 is that program.

**Nothing at all.** `warn.c` ends with `strlen(code)` on that three-byte array and neither
compiler says a word: the declaration was diagnosable, the read is not, once a function
boundary is in the way. Nothing is said about `char b[3] = {'a','b','c'}` until something
reads it, about `sizeof p` once `p` is a parameter, or about `letter[c]` while the bytes
happen to stay below 0x80. A warning is evidence of a defect; no warning is evidence of
nothing, and most of this unit's defects wait for a particular byte to arrive.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| string vs. character sequence | A string ends in a zero byte. A character sequence is just bytes. Only the first may be passed to `strlen`, `strcpy` or `%s`. |
| length vs. size | Length counts the bytes before the terminator (`strlen`). Size is the whole buffer (`sizeof`, or a `cap` parameter). A string fits when `len < cap`. |
| `sizeof` vs. `strlen` | `sizeof` is the type's storage, known at compile time, correct even for garbage bytes. `strlen` is a measurement of the data, and undefined if there is no terminator. |
| `sizeof` an array vs. a pointer | The array's bytes; the pointer's 8. The difference appears the moment the array is passed to a function. |
| `'\0'` vs. `'0'` vs. `NULL` | The byte 0; the character zero, which is 48; a null pointer. |
| an array vs. what it decays to | `a` and `&a[0]` are one address with two types; only `sizeof`, `&` and a literal initialiser see the difference. |
| `char` vs. `signed char` vs. `unsigned char` | Three distinct types. Plain `char`'s signedness is the platform's choice, so a data byte belongs in `unsigned char`. |

## `man` pointers

- `man 3 strlen`: SYNOPSIS for the `const` and the return type, DESCRIPTION for "excluding the
  terminating null byte". On Linux, `man 3 strnlen` is a separate page; on macOS it shares the
  `strlen(3)` page.
- `man 3 puts`: shared with `fputc`, `putc`, `putchar` and `fputs` on both systems.
  DESCRIPTION has the one sentence that separates `puts` from `fputs`; RETURN VALUE has the
  "nonnegative number" wording.
- `man 7 string_copying` (Linux only): the **Terms** section, which defines string, character
  sequence, length, size, end and the copy/catenate/chain vocabulary this course uses.
- `man 3 string` (both): an index of the whole family by job — Copy, Catenate, Compare,
  Search, Separate. The fastest way to find the page of a function whose name you half
  remember; `man -k` (`apropos`) finds the rest.

## Check yourself

1. `char s[] = "hi\0there";` What are `sizeof s` and `strlen(s)`?
2. A function receives `char *buf`. Why can it not find out how many bytes `buf` points at?
3. Write the condition that is true exactly when a string of length `len` fits in a buffer of
   size `cap`, and say which of the two counts the terminator.
4. `char n[4] = {'2','0','2','6'};` Is `n` a string? What does `printf("%s", n)` do?
5. `signed char c = (signed char)0xC3;` What is `c > 0`, and what is `(unsigned char)c`?
6. A 256-entry table is indexed by a byte from a file. Write the subscript so that it is
   correct for every byte, and say what goes wrong without the cast.

<details><summary>Answers</summary>

1. `sizeof s` is 9 (`h i \0 t h e r e \0`: eight characters-or-escapes plus the terminator the
   compiler appends); `strlen(s)` is 2, because it stops at the embedded zero byte.
2. The array type decayed to a pointer at the call, and a pointer carries no size. The only
   ways to know are a `cap` parameter the caller filled in with `sizeof`, or a terminator the
   function can search for.
3. `len < cap`. `cap` counts the terminator; `len` does not.
4. No: four characters in four bytes, no terminator. `printf("%s", n)` reads past the array —
   undefined behaviour, and a `stack-buffer-overflow` READ under ASan.
5. `c > 0` is false (`c` promotes to -61); `(unsigned char)c` is 195. The bits never moved.
6. `table[(unsigned char)b]`. Without the cast a byte above 0x7F promotes to a negative `int`
   on this platform and the subscript is outside the table: undefined behaviour, a wrong
   answer in practice, a `global-buffer-overflow` under ASan.

</details>

## Takeaways

- A C string is a convention — bytes then a zero byte — and no part of the language or library
  checks it; the code that writes the bytes is the only thing that can.
- `sizeof` is a compile-time fact about a type and `strlen` is a run-time measurement of data;
  for a pointer, `sizeof` tells you nothing about the text.
- A string of length `len` needs `len + 1` bytes, so it fits when `len < cap`, and `cap`
  always includes the terminator.
- A string literal is the compiler's read-only array: point at it with `const char *` and copy
  it before changing a byte.
- A full fixed-width field is a character sequence, not a string, and needs a bounded reader
  (`strnlen`, `memchr`, `%.*s`).
- A byte of data belongs in an `unsigned char`; in plain `char` it may promote to a negative
  `int` and turn a comparison, a shift or a subscript into a defect.
- A READ overflow in a string function means a missing terminator, and the line to fix is the
  one that wrote the bytes, not the one the report names — which is the habit the whole course
  is built on.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-literal-sizes/` | `[PREDICT]` | | `sizeof` and `strlen` for fourteen declarations |
| e02 | `e02-count-and-terminate/` | `[WRITE]` | S | length, end, "is it terminated", and a bounded fill, with no library call |
| e03 | `e03-literal-write/` | `[BUG]` | | a correct in-place function handed a string literal; fix the caller |
| e04 | `e04-missing-nul/` | `[REPORT]` | | a `stack-buffer-overflow` READ inside `strlen`: find the declaration from the report |
| e05 | `e05-char-signedness/` | `[PREDICT]` | | the byte 0xE9 in three types: values, comparisons, casts, shifts, subscripts |
| e06 | `e06-decay-trace/` | `[TRACE]` | | draw the storage for an array, a pointer, an array of pointers and an array of arrays |
| e07 | `e07-string-lookup/` | `[LOOKUP]` | | nine facts about `strlen` and `puts` from the manual pages, with quotes |

## Drills

| # | Folder | Skill | Target |
|---|---|---|---|
| r01 | `drills/r01-walk-reps/` | five questions about a string, each by one pointer walk | 8 min |
| r02 | `drills/r02-init-reps/` | fill a buffer by hand, always terminated, never past `cap` | 10 min |
| r03 | `drills/r03-escape-reps/` | render a buffer so that `\0`, `\n` and `\xHH` are visible | 15 min |
