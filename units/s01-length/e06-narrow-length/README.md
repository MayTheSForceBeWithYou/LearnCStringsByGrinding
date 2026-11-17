# s01-e06-narrow-length `[BUG]`

## Goal

A record stores a name and, as its wire format requires, the name's length in a single byte.
Long names are supposed to be rejected. One of them is not, and the record ends up describing
itself wrongly, with no crash, no sanitizer report and no compiler warning. Find the defect,
fix it, and explain why no tool said anything. Afterwards you know what to check at every
boundary where a `size_t` is stored in something narrower — which is every protocol, every
file format and every on-disk struct.

## Background

Formats store lengths in fixed-width fields: one byte, two bytes, a 32-bit word. That is not a
mistake; it is how a reader of the format knows where a field ends without scanning for a
terminator, and it is why a record can be read with one `memcpy`. The format in this exercise
uses one byte, so it can describe names of 0 to 255 characters, and `name.h` says so.

The mistake is at the boundary. `strlen` returns a `size_t`, and storing a `size_t` in an
`unsigned char` is a **conversion**, which C defines rather than diagnoses: for unsigned
types, the value is reduced modulo 256 (C17 6.3.1.3p2). So 300 becomes 44, 256 becomes 0, and
255 stays 255. The program keeps running with a record whose length field says 44 and whose
buffer holds 300 characters, and every later reader believes the 44.

This is the defect class that no tool catches. There is no out-of-bounds access: the buffer is
512 bytes and the name fits in it. There is no undefined behaviour: the conversion is defined,
and UBSan's integer checks are about signed overflow and division, not about a narrowing
conversion that loses information. The compiler does not warn either, because such conversions
are everywhere in correct C code, and `-Wconversion` (which would warn) is not in `-Wall
-Wextra` and is far too noisy to turn on. What is left is a *check*: the code has to compare
the value against what the field can hold, before it converts.

The lesson to carry: **a narrower type needs a range test at the conversion, and the test
belongs with the conversion, not near it.** The bound to test against is what the *field* can
describe, which is not the same number as the size of the buffer.

Read **Foundations: a length that does not fit its type** in `LESSON.md` before you start.

## Symptom

`make check` fails in the plain build, with no crash and nothing from the sanitizers. From
`capture.txt`, produced from exactly this source:

```
FAIL main.c:35: name_set rejects a name longer than NAME_MAX
FAIL main.c:36: a rejected name changes nothing in the record
FAIL main.c:35: name_set rejects a name longer than NAME_MAX
FAIL main.c:36: a rejected name changes nothing in the record
FAIL main.c:35: name_set rejects a name longer than NAME_MAX
FAIL main.c:36: a rejected name changes nothing in the record
FAIL main.c:56: a rejected name leaves the stored name alone
s01-e06-narrow-length: 7 check(s) failed
```

Three names are rejected by the specification and accepted by the code: 256, 300 and 511
characters. Each failure is two checks — the return value, and the record's contents — and the
last line is the consequence: a record that was holding a good name before the rejected call
is not holding it afterwards.

`make asan` prints the same seven lines and **no report**. That is the interesting part of this
exercise, and the third heading of `ANSWER.md` asks you to explain it.

## What the code is supposed to do

`name.h` is the specification, and it is correct. Read the two constants carefully: they are
different numbers and they mean different things.

- `NAME_CAP` (512) is the size of the record's buffer. One buffer is reused for every record,
  so it is deliberately larger than any name the format allows.
- `NAME_MAX` (255) is **the longest name the one-byte length field can describe**.
- `name_set(n, s)` copies `s` into the record and stores its length, returning 0. If the name
  cannot be stored it returns -1 and **changes nothing at all** in the record.
- `name_len(n)` returns the stored name's length, from the record's own length field.
- `name_get(n, dst, cap)` copies the name out using that length field, with the `snprintf`
  contract: always terminated when `cap > 0`, return value is the name's length, so
  `ret >= cap` means truncated.

| `s` | `name_set` | `name_len` afterwards |
|---|---|---|
| `""` | `0` | 0 |
| `"ada"` | `0` | 3 |
| 100 characters | `0` | 100 |
| 254 characters | `0` | 254 |
| 255 characters | `0` | 255 |
| 256 characters | `-1` | unchanged |
| 300 characters | `-1` | unchanged |
| 511 characters | `-1` | unchanged |

## What to hand in

1. **The fix**, in `name.c`. There is exactly one defect, in one function, and nothing else in
   the exercise is wrong. Do not change `main.c` or `name.h`: the format is not negotiable,
   and widening the length field would be a different program. Do not change `name_len` or
   `name_get`, which are the innocent readers.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted, and which of the two constants in
     `name.h` the test should have used.
   - *The mechanism*: for a 300-character name, what `strlen` returns, what ends up in `len`,
     and why that number; then what `name_len` and `name_get` report afterwards, and which
     bytes of `buf` are now unreachable through the record's own API.
   - *How the tool showed it*: it did not. Say why ASan, UBSan and the compiler are all
     silent, which check caught it instead, and what a program with no such check would have
     shipped.
   - *The rule you take away*: one sentence.

## Constraints

- Names of 255 characters and fewer must still be stored, exactly as before: the fix must not
  cost the format a single value it is entitled to.
- A rejected name leaves the record byte-for-byte unchanged, including the buffer. Test the
  name's length **before** copying anything.
- The record stays the size it is: `unsigned char len` is part of the format.
- No allocation.

## Verify

```bash
make check
```

Done means `s01-e06-narrow-length: all checks passed` in the plain stage and again in the ASan
stage, and `== check: OK`.

A good experiment afterwards, in a scratch file outside this directory: print
`(unsigned char)300`, `(unsigned char)256` and `(unsigned char)255`, and then compile the same
file with `-Wconversion` and read what the compiler has to say about each of them. That flag
is why the defect is possible and why nobody turns it on: it is right about these three lines
and noisy about a hundred correct ones.

<details><summary>Hint</summary>

`name.h` defines two constants. The code tests against one of them.

</details>
