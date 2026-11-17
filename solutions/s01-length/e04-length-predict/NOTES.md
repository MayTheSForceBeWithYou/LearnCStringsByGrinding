# s01-e04-length-predict: notes

## Why each answer is what it is

| Key | Answer | Reason |
|---|---|---|
| `a.strlen` | `5` | `hello` is five characters before the terminator |
| `a.sizeof` | `6` | the array was sized from the literal, terminator included |
| `a.strnlen0` | `0` | `maxlen` is 0, so no byte is read and the answer is 0 |
| `a.strnlen3` | `3` | three bytes, no NUL among them: the answer is `maxlen` |
| `a.strnlen6` | `5` | the terminator is the sixth byte, inside the bound, so a true length |
| `e.strlen` | `2` | `strlen` stops at the first NUL, at index 2 |
| `e.sizeof` | `6` | `a b \0 c d \0`: the literal's own NUL plus the one the compiler appends |
| `e.tail_strlen` | `2` | `&e[3]` is `"cd"`, a perfectly good string |
| `f.strlen` | `2` | the text is `hi`; everything after it is zero |
| `f.sizeof` | `8` | the explicit size wins |
| `f.strnlen8` | `2` | the terminator is found at index 2, well inside the bound |
| `p.strlen` | `3` | `p` is `&a[2]`, so the string from there is `"llo"` |
| `p.sizeof` | `8` | `sizeof` measures the pointer, not the text |
| `full.sizeof` | `4` | four `char`s |
| `full.strnlen4` | `4` | no NUL in four bytes, so the answer is the bound |
| `full.strlen` | `undefined` | there is no terminator, so the call has no defined result |

## Numbers worth noticing

The three `strnlen` answers for `a` are 0, 3 and 5, and they come from three different
reasons: the bound was zero, the bound was reached, and the terminator was found. A reader of
the return value alone cannot tell the second from the third — both are "the number of bytes
before a NUL, or the bound" — which is why the useful test is `ret == maxlen`, meaning "no
terminator in there", rather than a comparison with the length you expected.

`sizeof` is 6 for both `a` and `e`, and their contents are completely different. `sizeof`
never knows anything about the text; it is the same number for `"hello"` and for
`"ab\0cd"` because both initialisers are six bytes long.

`f.sizeof` is 8 and `f.strlen` is 2, and the useful number for a copy into `f` is neither: it
is 7, `sizeof f - 1`, the longest string `f` can hold. Every bounded copy in S03 takes
`sizeof f` and works out the 7 itself.

`full.strlen` is the only answer that is not a number, and the only one the program refuses to
compute. It would have produced *a* number on any given run — on this machine, probably 4 or 5
— and that number is an accident of what lies after the array. "Undefined" is a statement
about what the standard guarantees, not a prediction about one run.

## Alternatives

None: these are facts about the declarations and the calls. The alternative is in how you
derive them, and the method that does not slip is to draw the object before predicting
anything:

```
a:      h  e  l  l  o \0                 6 boxes, terminator at index 5
e:      a  b \0  c  d \0                 6 boxes, terminators at 2 and 5
f:      h  i \0 \0 \0 \0 \0 \0           8 boxes, terminator at index 2
full:   A  1  2  3                        4 boxes, no terminator anywhere
p:      ----> a[2]                        a pointer; its own size is 8
```

With the boxes drawn, `sizeof` is "how many boxes", `strlen` is "the index of the first `\0`",
and `strnlen(s, n)` is "the index of the first `\0`, or `n` if there is none in the first `n`
boxes". All sixteen answers read straight off the picture.

## The usual mistake

`a.strnlen3 = 5`: answering with the string's length because "the string is five characters
long". `strnlen` was given three bytes to look at and found no terminator in them, so the
only honest answer is 3. The function cannot see the rest.

`e.strlen = 5`: counting all the text in the object. There are five characters in those six
bytes, and a terminator sits between the second and the third, so `strlen` reports 2. The `cd`
is unreachable through any `<string.h>` function given `e` alone.

`p.sizeof = 4`: answering for the text (`"llo"` is four bytes with its terminator) rather than
for the pointer. This is the `sizeof`-on-a-pointer bug in its purest form, and in real code it
is `snprintf(buf, sizeof buf, ...)` where `buf` is a `char *`.

`full.strlen = 4`: answering what the call would probably print. It reads `full[4]`, which is
not part of `full`, and under ASan the program dies rather than returning 4.

## How libc does it

`strlen` and `strnlen` are described in the previous exercise's notes: vectorised scans, a
word or more at a time. What matters for this exercise is what they are *given*: in every one
of these sixteen expressions, the function receives a pointer and nothing else (or a pointer
and a bound the caller chose). There is no mechanism by which `strlen` could know that `full`
has four bytes, and no mechanism by which `sizeof` could know that `p` points at three
characters.

That is the whole reason the C library has two families — the `str*` functions that trust a
terminator and the `mem*`/`*n*` functions that take a count — and the reason every careful C
interface passes a buffer and its capacity together. `sizeof` is the one place where the
compiler still knows the size, and it loses that knowledge at the first function call.
