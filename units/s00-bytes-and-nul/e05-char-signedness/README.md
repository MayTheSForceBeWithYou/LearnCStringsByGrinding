# s00-e05-char-signedness `[PREDICT]`

## Goal

One byte, 0xE9, held in a `signed char`, an `unsigned char` and a plain `char`. Predict
nineteen expressions involving the three: their values, six comparisons, two casts, two
shifts and two array indexes. Afterwards you can look at any expression containing a `char`
and say what integer it really becomes, which is the difference between code that handles
text above ASCII and code that corrupts it.

## Background

`char` is an integer type one byte wide, and the standard deliberately does not say whether
it is signed (C17 6.2.5p15: "the implementation shall define `char` to have the same range,
representation, and behavior as either `signed char` or `unsigned char`"). On x86-64 Linux
and on macOS it is signed; on ARM Linux, on AIX and on s390x it is unsigned. Code that works
on your machine and corrupts text on your colleague's usually turns on this one sentence.

Signed or not, a `char` almost never stays a `char` in an expression. The **integer
promotions** (C17 6.3.1.1p2) convert any type narrower than `int` to `int` before anything
happens: before a comparison, before arithmetic, before being passed to a variadic function,
before being used as an array subscript. So the question is never "what is in the byte" but
"what `int` does this byte become".

For a byte below 0x80 the answer is the same either way, which is why this defect hides for
years in a code base that has only ever seen ASCII. For 0xE9 the two answers are 233 and -23,
and everything downstream differs:

| | `unsigned char` | `signed char` (and plain `char` here) |
|---|---|---|
| the `int` it promotes to | 233 | -23 |
| `> 0` | true | false |
| `< 'a'` | false | true |
| as an array subscript | 233, inside a 256-entry table | -23, outside the object |
| `>> 4` | 14, the high nibble | -2, and the sign bit copied in |

The last row of that table is a real bug class, not a curiosity: a 256-entry lookup table
indexed by a plain `char` reads memory before the table whenever the data has a byte above
0x7F. That is S07 e02, and it is exactly what `isalpha(c)` does when `c` is a plain `char`
holding such a byte, which is why every `<ctype.h>` call in this course is written
`isalpha((unsigned char)c)`.

Two details of the C grammar also appear here. A **character constant** such as `'\xE9'` has
type `int`, and its value is the value of the corresponding `char` (C17 6.4.4.4p10): on this
machine, -23 rather than 233 — so `'\xE9'` does not compare equal to the byte you would
expect. And a cast between `signed char` and `unsigned char` does not move any bits; it
reinterprets the same eight bits, and the resulting `int` differs by 256.

Read **Foundations: one byte, three types** in `LESSON.md` before you start. The rule it ends
with is the one to remember: when a byte is data, keep it in an `unsigned char`, and cast it
to `unsigned char` at every point where it becomes an `int`.

## The setup

`main.c` declares

```c
signed char sc = (signed char)0xE9;
unsigned char uc = (unsigned char)0xE9;
char pc = (char)0xE9;
```

and a 256-entry table `int letter[256]`, filled with 1 at the indexes of the 52 ASCII letters
and 0 everywhere else. The three comparisons are made through small functions `eq(a, b)`,
`lt(a, b)` and `gt(a, b)`, each taking two `int` parameters, so that every comparison really
happens at run time on promoted values and nothing is folded into a constant by the compiler.

| Key | Expression |
|---|---|
| `sc.value`, `uc.value`, `pc.value` | the value of each variable, as `printf("%d", ...)` shows it |
| `pc.is_signed` | `CHAR_MIN < 0`: is plain `char` signed on this machine? |
| `sc.gt_zero`, `uc.gt_zero` | `gt(sc, 0)`, `gt(uc, 0)` |
| `sc.eq_uc` | `eq(sc, uc)` |
| `sc.eq_charconst`, `uc.eq_charconst` | `eq(sc, '\xE9')`, `eq(uc, '\xE9')` |
| `sc.lt_a`, `uc.lt_a` | `lt(sc, 'a')`, `lt(uc, 'a')` |
| `sc.to_uchar`, `uc.to_schar` | `(unsigned char)sc`, `(signed char)uc` |
| `sc.shift4`, `uc.shift4` | `sc >> 4`, `uc >> 4` |
| `index.sc`, `index.uc` | the value the subscript uses in `letter[sc]` and `letter[uc]` |
| `letter.at_uc` | the value `letter[uc]` reads |
| `letter.at_sc` | the value `letter[sc]` reads |

The program evaluates all of them except the last. It does not evaluate `letter[sc]`, and
that is the point of the last question.

## How to write an answer

Every answer is one decimal integer, except `letter.at_sc`, which may be a number or the word
`undefined`. A question that asks whether something is true wants `1` for true and `0` for
false, which is what the operators themselves produce.

Two examples that are not part of the graded set:

| Expression | Answer | Why |
|---|---|---|
| `uc + 1` | `234` | `uc` promotes to the `int` 233, and `233 + 1` is 234 |
| `(int)'A'` | `65` | a character constant already has type `int` |

Negative answers are written with a leading minus and no space: `-23`.

## Constraints

- Fill in all nineteen lines before the first run. Work each expression out by writing down
  the `int` each operand promotes to first, then applying the operator to those two `int`s.
- No compiler, no `printf`, no debugger while predicting. `limits.h` and the lesson are
  allowed.
- Three of the answers depend on the machine (`pc.value`, `pc.is_signed`, and one more that
  follows from them). Answer for the machine you are on: on x86-64 Linux and on macOS, plain
  `char` is signed. If you are grading this on a platform where it is not, your answers
  should change, and so will the grade — that is the portability point made concrete.

## What the program checks

For every key: `ok`, `WRONG`, or `blank`. It never prints the correct value. Eighteen of the
nineteen are computed by evaluating the expression; `letter.at_sc` is compared by a hash,
because the program will not perform that read. `make check` passes only when all nineteen
are `ok`, in both the plain and the ASan build.

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with

```
s00-e05-char-signedness: 0 of 19 answers correct
```

When you are done it ends with `s00-e05-char-signedness: all checks passed`.

Afterwards, in a scratch file outside this directory, write the four-line program that does
perform `letter[sc]`, build it with `make asan`-style flags, and see what AddressSanitizer
calls it. Then write the version of the same lookup that is correct for every byte, and
notice that the fix is one cast in one place.

<details><summary>Hint</summary>

Do the three `.value` answers first; after that, every comparison is a comparison of two
numbers you have already written down.

</details>
