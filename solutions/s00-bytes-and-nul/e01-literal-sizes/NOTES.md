# s00-e01-literal-sizes: notes

## Why each answer is what it is

| Declaration | `sizeof` | `strlen` | Reason |
|---|---|---|---|
| `char a[] = "abc";` | 4 | 3 | the array is sized from the initialiser, terminator included |
| `const char *p = "abc";` | 8 | 3 | `sizeof` measures the pointer; the four bytes are elsewhere |
| `char b[3] = {'a','b','c'};` | 3 | undefined | three characters, no terminator: not a string |
| `char c[8] = "abc";` | 8 | 3 | the explicit size wins; bytes 4 to 7 are zero by C17 6.7.9p21 |
| `char d[] = "ab" "cd";` | 5 | 4 | adjacent literals are one literal at translation phase 6 |
| `char e[] = "a\0b";` | 4 | 1 | four bytes: `a`, NUL, `b`, NUL. `strlen` stops at index 1 |
| `char f[] = "";` | 1 | 0 | the empty string is one byte, and that byte is the terminator |
| `char g[] = {'h','i'};` | 2 | undefined | no terminator, so no string |
| `char h[] = {'h','i','\0'};` | 3 | 2 | the same two characters, terminated by hand |
| `char m[3][8] = {...};` | 24 | 2 | three rows of eight bytes; `m[1]` is `"cd"` |
| `const char *v[] = {...};` | 24 | 3 | three pointers of 8 bytes; `v[2]` points at `"ccc"` |
| `const char *q = &"hello"[2];` | 8 | 3 | a pointer into the middle of a literal: `"llo"` |
| `char r[] = "caf\xC3\xA9";` | 6 | 5 | `c a f` and two bytes from the two `\x` escapes, then the terminator |
| `char s[10] = "hi";` | 10 | 2 | the explicit size wins; bytes 3 to 9 are zero |

## Numbers worth noticing

Four declarations hold a byte the initialiser never mentions: `a`, `d`, `e` and `f` get a
terminator the compiler supplies, and `c` and `s` get that plus zero padding to their stated
size. Two (`b`, `g`) hold exactly what was written and nothing else, which is why they are
not strings.

`sizeof` is 8 four times (`p`, `q`, and each element of `v`; `sizeof v` is 3 × 8), and in
every one of those cases it is a fact about a pointer and tells you nothing about the text.
Passing `sizeof p` as a capacity is the `sizeof`-on-a-pointer bug, and it is S18 e04.

`r` is the one row where a character and a byte differ: `"caf\xC3\xA9"` is five bytes of
which the last two are one character, `é`, in UTF-8. `strlen` says 5 because `strlen` counts
bytes. Nothing in `<string.h>` counts characters; S15 and S16 are about the difference.

## Alternatives

None: these are facts about the declarations. The alternative is in how you derive them. Draw
the boxes, one per byte, and fill in the terminator explicitly; then `sizeof` is the number of
boxes and `strlen` is the index of the first `\0`.

A useful check on your own answer: `sizeof` of an array of `char` is always at least
`strlen` + 1 when the object is a string, and exactly that when the array was sized from its
initialiser.

## The usual mistake

`p.sizeof = 4`: answering for the literal rather than for the pointer. The literal really is
4 bytes, but `sizeof` is applied to `p`, whose type is `const char *`. This is the mistake
that makes `char *buf = malloc(n); snprintf(buf, sizeof buf, ...)` write 8 bytes into a
1000-byte buffer and never report trouble.

`e.strlen = 3`: reading `"a\0b"` as three characters. It is three characters *in the source*,
but the middle one is a NUL byte, and `strlen` is defined to stop at the first NUL. The
bytes after a terminator are perfectly real; no library function in `<string.h>` will look at
them.

`b.strlen = 3`: answering what the call would probably print rather than what the standard
says. The object has no NUL, so `strlen` reads `b[3]`, which is not part of `b`. Whatever
number comes out is an accident of the stack frame, and under ASan the program dies instead.

## How libc does it

`strlen` is not a loop over bytes in any libc you will link against. glibc and Apple libc read
an aligned machine word (or a 16- to 64-byte vector) at a time and test the whole block for a
zero byte with a bit trick or a vector compare, then locate the byte inside the block. That is
why `strlen` on a long string is far faster than one byte per iteration, and why it is allowed
to read a few bytes past the terminator inside the same aligned block: libc knows its page
boundaries, and a portable C program does not get to make that trade.

`sizeof` is not in libc at all. It is computed by the compiler from the type, and the compiled
program contains only the resulting constant. That is why it costs nothing and why it cannot
possibly know about a terminator.
