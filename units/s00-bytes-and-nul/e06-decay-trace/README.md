# s00-e06-decay-trace `[TRACE]`

## Goal

Draw where the bytes are, for the four ways C gives you to hold text: an array, a pointer to a
literal, an array of pointers, and an array of arrays. Then say, for every expression over
them, what it denotes, what its type is and what `sizeof` gives. Afterwards you can read a
declaration and know immediately which of the four you are looking at, which matters because
all four are written `char` with some punctuation and only two of them own their bytes.

## Background

C has one syntax for text and several different objects behind it:

```c
char a[] = "abc";                        /* an array of 4 bytes, yours, writable       */
const char *p = "abc";                   /* a pointer; the 4 bytes are the compiler's  */
const char *v[3] = {"a", "bb", "ccc"};   /* 3 pointers of your own, to 3 literals      */
char m[3][8] = {"ab", "cd", "ef"};       /* 24 bytes, yours, in 3 rows of 8            */
```

`a` and `p` behave identically in almost every expression, because of **array-to-pointer
conversion** (C17 6.3.2.1p3): an array used in an expression becomes a pointer to its first
element. (The program measures that pointer through a variable, `char *ap = a;`, rather than
by writing `sizeof(a + 0)`, which clang rejects under `-Wsizeof-array-decay` — asking for the
size of a decayed array is nearly always a mistake.) So `a[1]`, `*(a + 1)`, `p[1]` and `*(p + 1)` are all the same kind of thing, and a
function that takes a `char *` cannot tell which it was given. Three places do not convert,
and they are where the difference lives: the operand of `sizeof`, the operand of `&`, and a
string literal used to initialise an array.

That is the whole lesson of this exercise. `sizeof a` is 4 because `a` is an array of 4 bytes;
`sizeof p` is 8 because `p` is a pointer. The *text* is 4 bytes in both cases, and `sizeof`
tells you about the text in only one of them. Every "`sizeof` on a pointer" bug in C is this
sentence, and the only reason `sizeof buf` is a safe idiom for a capacity is that `buf` is an
array *at that point in the source*: pass it to a function and the information is gone.

`m` is the one that surprises people twice. Its rows are arrays, so `m[1]` is an array of 8
bytes and `sizeof m[1]` is 8 — but a pointer is also 8 bytes on both course platforms, so the
two 8s in the printout mean different things. And `m` is a single block of 24 bytes with the
rows laid out end to end, which is why `&m[1][0] - &m[0][0]` is exactly 8. An array of
pointers (`v`) has no such property: its three pointers are side by side, and the strings they
point at are wherever the linker put them.

Read **Foundations: where the bytes live** in `LESSON.md` and the `sizeof` card first.

## What to do

```bash
make check
```

The program prints three tables about the four declarations above — the `sizeof` and type of
twelve expressions, a set of lengths and offsets, and which bytes may be written — and then
asserts every identity the tables claim. Record what it prints in the template below, and
draw the storage. Nothing to write in a file the harness reads: the deliverable is the filled
template, in your notes.

## Answer template

The first row of each part is filled in. The byte boxes use the course notation: a character
as itself, the terminator as `\0`, and `#` for a byte nothing has written. In the diagrams,
an arrow means "holds the address of".

**1. The four objects.** How many bytes does each one own, and what is in them?

```
char a[] = "abc";
  a:     a   b   c   \0                     4 bytes, in the frame of main, writable
         0   1   2   3

const char *p = "abc";
  p:     [ an address ]  ──→  a b c \0      ___ bytes in p, ___ bytes in the literal
                                            the literal is in ____________ storage

const char *v[3] = {"a", "bb", "ccc"};
  v:     [ ___ ] [ ___ ] [ ___ ]            ___ bytes in v
           │       │       │
           ↓       ↓       ↓
         ______  ______  ______             ___ + ___ + ___ bytes of literals

char m[3][8] = {"ab", "cd", "ef"};
  m:     ________ ________ ________         ___ bytes in m, in ___ rows of ___
         m[0]     m[1]     m[2]             every byte not initialised is ____
```

**2. The twelve expressions.** Fill in from the first table the program prints.

```
expression     type              sizeof   what it denotes
a              char[4]                4   the four bytes a b c \0
&a[0]          ____                ____   ____
ap = a         ____                ____   ____
*a             ____                ____   ____
p              ____                ____   ____
*p             ____                ____   ____
v              ____                ____   ____
v[0]           ____                ____   ____
*v[2]          ____                ____   ____
m              ____                ____   ____
m[1]           ____                ____   ____
m[1][0]        ____                ____   ____
```

**3. Lengths and offsets.**

```
strlen(a)             = 3        the characters before the terminator
strlen(p)             = ____
strlen(m[1])          = ____     and sizeof m[1] = ____  (the two numbers differ: why?)
&a[3] - &a[0]         = ____
&m[1][0] - &m[0][0]   = ____     so the rows are ____
&m[2][0] - &m[0][0]   = ____
```

**4. The four questions to answer in prose**, two sentences each:

```
(a) Which of the four declarations own their text, and which only point at someone else's?

(b) For which of them is sizeof the right expression for a buffer capacity, and why is it
    wrong for the others?

(c) a and &a[0] are the same address. Name the two things that are different about them,
    and one expression in which the difference shows.

(d) m[1] and v[1] are both written "the second one". What is each one's type, how many
    bytes does each denote, and which of the two can be made to point somewhere else?
```

## Constraints

- Do not change `main.c`. Record what it prints; the program is the authority, not your
  memory of what `sizeof` does.
- Draw the boxes by hand before reading the second table. The point is to find out whether
  your picture of the storage was right, and that only works in that order.
- Answer (b) and (d) without running anything else. If you are tempted to test, write the
  prediction down first.

## Verify

```bash
make check
```

The program prints the three tables and ends with `s00-e06-decay-trace: all checks passed`,
then prints them again under ASan and UBSan, and `== check: OK`. The checks are the
program's own claim that the tables are true; if one ever fails, the table above it is wrong
and this README is wrong with it.

Two of the printed lines are statements rather than demonstrations: the program says that
`*p = 'X'` and `*v[0] = 'z'` are not allowed, and does not do them. e03 is what happens when
a program does.
