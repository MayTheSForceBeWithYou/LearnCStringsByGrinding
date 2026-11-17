# s01-r01-strlen-reps: notes

## Why it is correct

`len_index` and `len_diff` are the same walk with different state: an index that counts, and a
pointer whose distance from the start *is* the count. Both test the byte before using it, so
the terminator is read and nothing after it is.

`len_bounded` has two reasons to stop and the order decides whether it is safe:

```c
while (n < maxlen && s[n] != '\0')
    n++;
```

`&&` does not evaluate its right operand when the left is false (C17 6.5.13p4), so `s[maxlen]`
is never read. That is what makes the function usable on a field with no terminator, and it is
the only line in the drill where the order of two conditions is the whole point.

`len_fits` is one comparison, and it is built on the bounded function so that it inherits the
bound:

```c
return len_bounded(s, cap) < cap;
```

Read it as: measure at most `cap` bytes; if the answer came back smaller than `cap`, a
terminator was found inside the buffer, so the string and its terminator fit. If the answer
equals `cap`, either the string is exactly `cap` characters (needing `cap + 1` bytes) or it is
longer — in both cases it does not fit, and in both cases the loop stopped at the bound
without walking the rest. `cap == 0` gives `0 < 0`, which is false: nothing fits in no bytes.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | every loop's condition is false at once; `len_fits("", 1)` is `0 < 1` |
| `cap == 0` | `len_bounded(s, 0)` is 0, and `0 < 0` is false |
| an exact fit (`cap == len + 1`) | `len_bounded` finds the terminator; `len < cap` |
| one byte too small (`cap == len`) | the bound is reached; `cap < cap` is false |
| a string much longer than `cap` | the bound stops the walk: no wasted pass, no over-read |
| a full field, no terminator | `n < maxlen` fails first; `s[maxlen]` is never read |
| `maxlen == SIZE_MAX` | `maxlen` is only compared, never incremented |
| bytes above 0x7F | the test is `!= '\0'`, so the signedness of `char` never enters |

## Alternatives

| This function | The library |
|---|---|
| `len_index`, `len_diff` | `strlen(s)` |
| `len_bounded` | `strnlen(s, maxlen)`, or ISO C's `memchr(s, '\0', maxlen)` and a subtraction |
| `len_fits` | `strnlen(s, cap) < cap`, which is the same expression with the library's name |

`len_fits` written as `strlen(s) < cap` is the version most code contains, and it is correct
*only* when `s` is known to be a string. Given data from outside the program it walks the
whole input to answer a question about the first `cap` bytes, which is both slower and, on an
unterminated buffer, an over-read. The bounded form costs nothing extra and is right in both
cases, which is why it is the one to drill.

The pointer form of `len_bounded` (`const char *p = s; while (p < s + maxlen && *p) p++;`) is
equally good and has one trap: `s + maxlen` with `maxlen == SIZE_MAX` is pointer arithmetic
that overflows the object, which is undefined. The index form has no such problem, which is
why the bounded function is the one place in this drill where an index is better than a
pointer.

## The usual mistake

Writing `len_bounded` as "measure, then clamp" (`size_t n = len_index(s); return n < maxlen ?
n : maxlen;`). Every number it returns is right, and it reads past the end of any field that
has no terminator. The harness's exactly-sized field buffers turn that into an ASan `READ`
report, with your own function in frame `#0`.

Second: `len_fits` as `len_index(s) <= cap`. Two defects in one line — the unbounded
measurement, and `<=` instead of `<`, which forgets the terminator's byte and reports that a
5-character string fits in 5 bytes. The second is the classic off-by-one of C string handling,
and it is what S18 e01 is made of.

Third: `len_diff` returning `p - s` without the cast to `size_t`. The difference is a
`ptrdiff_t`, which is signed; returning it from a `size_t` function converts it anyway, so the
code is correct, but writing the conversion down is the habit that makes the signed/unsigned
boundary visible — and in a comparison rather than a return, the missing cast is a real bug.

## How libc does it

glibc's `strlen` tests a whole machine word, or a whole vector register, for a zero byte at a
time: one iteration per 8 to 64 bytes instead of one per byte. `strnlen` is a thin wrapper
over the vectorised `memchr`, which is why the Linux manual page defines it that way.

Two consequences for a hand-written loop. It will be several times slower than the library
call on long strings, so "I will just walk it myself" has a real cost in a hot path — and the
library versions are allowed to read past the terminator inside an aligned block they have
already touched, which your version is not. That asymmetry is exactly why this drill's harness
hands you buffers of their exact size: it holds your code to the rule the standard imposes on
it, not to the one libc gets to use.
