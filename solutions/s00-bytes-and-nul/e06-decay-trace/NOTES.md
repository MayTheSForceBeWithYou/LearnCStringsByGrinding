# s00-e06-decay-trace: notes

## Why it is correct

Every line the program prints is a `sizeof`, a `strlen`, or a difference of two pointers into
one object, and each is then asserted by a `check`. Nothing is measured that the standard does
not fix:

- `sizeof` is computed from the type at compile time, so `sizeof a` is 4 and `sizeof p` is
  the width of a pointer on the platform (printed in the header line rather than assumed to
  be 8).
- `&a[3] - &a[0]` is 3 because both are elements of the same array, which is the only case in
  which subtracting two pointers is defined (C17 6.5.6p9). The program never subtracts
  pointers into different objects, and never prints an address.
- `&m[1][0] - &m[0][0]` is 8 because a two-dimensional array is one block laid out row by row
  (C17 6.2.5p20: an array of arrays), and because the two expressions address elements of the
  same 24-byte object.
- The writable bytes are demonstrated by writing them and putting them back; the read-only
  ones are described, not attempted.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the two different 8s | the header prints the pointer width, and the table gives `m[1]` as `char[8]` |
| `m` is one block, not three | `&m[2][0] - &m[0][0] == 16`, asserted |
| rows are zero-filled past their text | `m[1][2] == '\0' && m[1][7] == '\0'`, asserted |
| `a` and `&a[0]` are the same address | `(char *)a == &a[0]`, asserted |
| a literal must not be written | stated and not performed; e03 performs it |
| a platform where a pointer is not 8 bytes | every pointer size is compared with `sizeof(char *)`, never with 8 |

## Alternatives

The same four declarations, written the way a program that must not care would write them:

```c
void f(const char *s, size_t cap);   /* the array's size cannot travel in the type */
```

Which is the whole reason capacities are passed as parameters in C, and why every function in
this course that writes into a buffer takes a `cap`. `sizeof` works only where the array
*declaration* is in scope, and that is one function at the top of the call chain.

For the array of pointers versus the array of arrays, the trade is the usual one: `v` costs a
pointer per entry and an indirection per access, and its strings can be any length and can be
shared; `m` costs a fixed row per entry whether it is used or not, and its rows can be written
in place. Code that sorts a table of names uses `v` (one comparator, pointers swapped); code
that writes fixed-width records uses `m` (no allocation, and the row width is the format).
S06 e04 sorts both and needs two different comparators, which is this difference showing up as
a cast.

## The usual mistake

Writing `sizeof p` as 4, because the literal is four bytes. The classic consequence:

```c
void save(const char *name) {
    char buf[64];
    /* ... */
    snprintf(buf, sizeof name, "%s.bak", name);   /* 8, not 64 */
}
```

which silently writes at most 7 characters and truncates everything, and the test suite that
uses short names never notices.

The second is reading `m[1]` as a pointer, because `m` "is" a pointer. `m` *converts* to a
pointer in most expressions, but `m[1]` is an array of 8 bytes, it cannot be assigned, and
`m[1] = "xy"` is not valid C. The rule to carry: an array converts to a pointer when it is
used as a value; it never becomes a pointer variable.

The third is expecting `v`'s strings to be next to each other because `v`'s pointers are. The
pointers are one object; the strings are three unrelated ones, and the only reliable statement
about their addresses is that each points to its own literal.

## How libc does it

Nothing in libc can tell these four apart, and that is the point worth taking away. Every
`<string.h>` function takes `char *` or `const char *`, so by the time `strlen`, `strcpy` or
`memcpy` sees your text, the array-ness is gone: no size, no bounds, no ownership. A function
can know a length only if the caller passed one, or if there is a terminator to find.

This is also why the two "safer" families in the library take their bound as a separate
parameter (`strncpy`, `snprintf`, `strnlen`) rather than deducing it. A deduced bound would
need the array type, which does not survive the call. The only tool C gives for carrying a
capacity across a function boundary is a second parameter, which is why `(buf, cap)` pairs
are the shape of every careful C interface, and why S19 is about writing that contract down.
