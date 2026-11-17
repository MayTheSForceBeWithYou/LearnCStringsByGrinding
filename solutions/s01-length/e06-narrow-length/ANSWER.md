# s01-e06-narrow-length: answer

## The defect

`name.c`, line 10, in `name_set`:

```c
    if (len + 1 > NAME_CAP)
```

The test is against `NAME_CAP` (512), the size of the buffer, when it should be against
`NAME_MAX` (255), the longest length the one-byte length field can describe:

```c
    if (len > NAME_MAX)
```

Both constants are in `name.h`, two lines apart, with comments saying which is which. The
buffer is not the constraint; the format is.

## The mechanism

For a 300-character name, `strlen(s)` returns 300 as a `size_t`. The test `len + 1 > NAME_CAP`
is `301 > 512`, which is false, so the name is accepted. `memcpy` copies 301 bytes into a
512-byte buffer — entirely legal, nothing out of bounds — and then:

```c
n->len = (unsigned char)len;      /* 300 stored in one byte */
```

The conversion to `unsigned char` is defined, not undefined: C17 6.3.1.3p2 says the value is
reduced modulo 2^8. 300 mod 256 is 44, so `len` holds 44. (256 would become 0, and 255 stays
255, which is why 255 is the last length the field can represent.)

Afterwards the record describes itself wrongly. `name_len` returns 44. `name_get` copies 44
bytes and terminates, so the name it hands back is 44 characters long. The remaining 256
characters are still in `buf`, and nothing in the record's API can reach them: every reader
uses the length field. The record's own terminator is at `buf[300]`, where `memcpy` put it, so
even a reader that ignored the length field and used `strlen` would disagree with `name_len`
by 256 characters.

## How the tool showed it

It did not, and this is the point of the exercise.

- **AddressSanitizer** reports accesses outside an object. Every read and every write here is
  inside `buf`: 301 bytes into 512. There is nothing to report.
- **UndefinedBehaviorSanitizer** reports undefined behaviour. The narrowing conversion is
  *implementation-independent and fully defined* for unsigned types, so there is nothing to
  report. (Had `len` been a `signed char`, the conversion would be implementation-defined, and
  UBSan still would not flag it.)
- **The compiler** could warn, under `-Wconversion`, which is not in `-Wall -Wextra` because
  narrowing conversions appear in a great deal of correct C. At `-O2` with the course's flags
  it says nothing.

What caught it was a **check**: `main.c:35` asserts that a 256-character name is rejected, and
`main.c:36` asserts that a rejected call leaves the record unchanged. Both come from the
header's own wording, which is why a specification written down as a contract is worth the
trouble.

A program with no such check would have shipped a record format whose length byte quietly
disagrees with its contents for every name longer than 255 characters. Downstream, that is a
name truncated to 44 characters in a report, a record whose next field is read from the wrong
offset by a reader that trusts the length, and — in a format where the length byte drives a
copy into a smaller buffer — the beginning of a security bug.

## The rule you take away

When a length is stored in a type narrower than `size_t`, test the value against the maximum
that type can represent, in the same expression that converts it.
