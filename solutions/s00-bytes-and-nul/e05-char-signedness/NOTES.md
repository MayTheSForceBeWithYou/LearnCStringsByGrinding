# s00-e05-char-signedness: notes

## Why each answer is what it is

The eight bits are `1110 1001` in all three variables. Nothing about the *bits* differs; what
differs is the `int` the integer promotions produce from them.

| Key | Answer | Reason |
|---|---|---|
| `sc.value` | `-23` | a signed 8-bit two's-complement byte: 233 - 256 |
| `uc.value` | `233` | an unsigned byte promotes to its own value |
| `pc.value` | `-23` | plain `char` is signed on x86-64 Linux and on macOS |
| `pc.is_signed` | `1` | `CHAR_MIN` is -128 here, so the test is true |
| `sc.gt_zero` | `0` | -23 > 0 is false |
| `uc.gt_zero` | `1` | 233 > 0 is true |
| `sc.eq_uc` | `0` | both promote first: -23 == 233 is false, although the bytes are equal |
| `sc.eq_charconst` | `1` | `'\xE9'` is an `int` whose value is `(char)0xE9`, so -23 == -23 |
| `uc.eq_charconst` | `0` | 233 == -23 is false |
| `sc.lt_a` | `1` | -23 < 97 |
| `uc.lt_a` | `0` | 233 < 97 is false |
| `sc.to_uchar` | `233` | the cast reinterprets the same bits as unsigned |
| `uc.to_schar` | `-23` | and back again |
| `sc.shift4` | `-2` | -23 >> 4 on a two's-complement machine keeps the sign: 0xFFFFFFE9 >> 4 is 0xFFFFFFFE |
| `uc.shift4` | `14` | 233 >> 4 is 14, which is the high nibble, `E` |
| `index.sc` | `-23` | the subscript is the promoted value |
| `index.uc` | `233` | likewise |
| `letter.at_uc` | `0` | index 233 is inside the 256-entry table, and 0xE9 is not an ASCII letter |
| `letter.at_sc` | `undefined` | `letter[-23]` is not an element of `letter`; C17 6.5.6p8 makes the pointer arithmetic itself undefined |

## Numbers worth noticing

`sc.eq_uc` is 0 while the two bytes are identical, and `sc.eq_charconst` is 1 while the two
*values* are 233 and -23 as anyone would read them. Both results are the promotions doing
their work: a comparison in C compares `int`s, and the only question is which `int` each side
became.

`sc.shift4` being -2 rather than 14 is the reason every hex dump in real code is written with
`unsigned char`: `"%02X", c >> 4` on signed data prints `FE` or worse. The portable form is
`(unsigned char)c >> 4`, or simply holding the data in `unsigned char` from the start.
Right-shifting a negative value is implementation-defined, not undefined (C17 6.5.7p5): every
platform this course runs on copies the sign bit, and none of them is wrong to.

`letter.at_sc` is the one answer that is not a number. The read is out of bounds, and that is
not softened by the table being a global, or by the memory before it being readable: the
standard's rule is about the object, not about the page. Built with the course's sanitizer
flags, UBSan reports the subscript as out of bounds for the array's type and ASan then reports
a `global-buffer-overflow` READ; without a sanitizer it quietly returns whatever integer
precedes the table, and a class table that answers "yes, a letter" for byte 0xE9 is a bug you will
look for in the parser rather than in the lookup.

## Alternatives

The one rule that removes every entry in this table: **data bytes live in `unsigned char`**.
If a buffer holds text or bytes that may be above 0x7F, declare it `unsigned char *`, or cast
at every point where a byte becomes an `int`:

```c
letter[(unsigned char)c]
isalpha((unsigned char)c)
printf("%02X", (unsigned char)c)
```

The cast is not defensive noise; it is the statement "this byte is a number from 0 to 255",
which is what a byte of text is. `<string.h>` already works this way internally: `strcmp` is
specified to compare bytes "as `unsigned char`" (C17 7.24.4), which is why `strcmp` orders
high bytes after ASCII while a hand-written `signed char` comparison puts them before it
(S06 e03).

The other half of the rule: when a byte is a *small number* rather than text, say so with
`uint8_t` or `int`, and never with plain `char`. Plain `char` means "a character of text",
and its signedness is the platform's business.

## The usual mistake

Answering 233 for `sc.value`, because the byte is 0xE9 and 0xE9 is 233. The bits say 233; the
type says how to read them, and `signed char` reads the top bit as a sign.

The other common one is `sc.eq_uc = 1`, on the grounds that the two variables hold the same
byte. They do. They are not compared as bytes: both are promoted to `int` first, by different
rules, and the comparison never sees a `char` at all. This is also why `memcmp` and `strcmp`
exist in their specified forms — comparing bytes correctly is fiddly enough that the library
fixes the rule rather than leaving it to the caller's types.

## How libc does it

Every byte-oriented function in the C library is specified in terms of `unsigned char`, which
removes the question entirely. `memset`'s value parameter is an `int` "converted to
`unsigned char`"; `memcmp` and `strcmp` compare bytes "as `unsigned char`"; `memchr` looks for
the byte "converted to `unsigned char`". So `memchr(s, 0xE9, n)` and
`memchr(s, (char)0xE9, n)` find the same byte, and `strcmp` gives the same order on every
platform whatever `char` happens to be.

`<ctype.h>` is the exception that proves the rule, and the reason is history: its functions
take an `int` that must be either `EOF` or "a value representable as an `unsigned char`"
(C17 7.4p1), because they were designed to be fed the return value of `getc`. Implementations
are allowed to be a 256-entry (or 257-entry) table lookup, and glibc's are exactly that, which
is why a negative argument indexes before the table. S07 is that whole story.
