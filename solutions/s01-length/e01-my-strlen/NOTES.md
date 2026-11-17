# s01-e01-my-strlen: notes

## Why it is correct

`my_strlen` walks a pointer to the terminator and returns the difference:

```c
const char *p = s;
while (*p != '\0')
    p++;
return (size_t)(p - s);
```

The loop tests the byte before using it, so the terminator is read (that is how the loop
knows to stop) and nothing after it is. The difference of two pointers into one object has
type `ptrdiff_t`, and the conversion to `size_t` is exact for any length that fits in an
object.

`my_strnlen` is the same walk with a second reason to stop, and the order of the two tests is
the whole exercise:

```c
while (n < maxlen && s[n] != '\0')
    n++;
```

`n < maxlen` comes first, so `s[maxlen]` is never evaluated. `&&` guarantees that: the right
operand is not evaluated when the left is false (C17 6.5.13p4), which here is not a style
preference but the reason the function is safe on a full field. Swap the two conditions and
the function reads one byte past the bound on every full field, returns the same numbers, and
passes every test that does not place the field at the end of an object.

Three facts fall out of the loop with no extra code. `maxlen == 0` reads nothing, because the
first test fails. `maxlen == SIZE_MAX` works, because `maxlen` is only compared and never
incremented. And a return value equal to `maxlen` means "no terminator found", because that is
the only way to leave the loop other than finding one.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | both loops' conditions are false at once; 0 |
| an embedded NUL | both stop at the first one: `*p != '\0'` / `s[n] != '\0'` |
| bytes above 0x7F | the test is `!= '\0'`, never `> 0`; the signedness of `char` never enters |
| a full field with no terminator | `n < maxlen` fails first, and `s[maxlen]` is never read |
| `maxlen == 0` | the same test, at `n == 0`: `s` is not dereferenced |
| `maxlen == SIZE_MAX` | nothing adds to `maxlen` |
| a string shorter than `maxlen` | the NUL test fails first, so the result is a true length |

## Alternatives

```c
size_t my_strnlen(const char *s, size_t maxlen)
{
    const char *p = memchr(s, '\0', maxlen);
    return p != NULL ? (size_t)(p - s) : maxlen;
}
```

That is how a real libc writes it, and `memchr` is forbidden here precisely because it is the
whole answer. It is worth knowing as the shape to reach for after S02: `memchr` is bounded,
vectorised, and reports "not found" with NULL, which is exactly the question `strnlen` asks.
The Linux manual page for `strnlen` states the same identity.

`my_strlen` can also be written with an index (`while (s[i] != '\0') i++;`), which compiles to
nearly the same code; drill r01 asks for both shapes on purpose, because the pointer form is
what makes `end_of` and the appender idiom of S04 natural.

What you must not do is define `my_strlen` as `my_strnlen(s, SIZE_MAX)`. It gives the right
answer, but the bound no longer means anything, and the function that was supposed to be the
bounded one is now the unbounded one wearing its name.

## The usual mistake

Measuring first and clamping afterwards:

```c
size_t n = my_strlen(s);          /* reads to the terminator, wherever that is */
return n < maxlen ? n : maxlen;   /* the number is right */
```

Every value this returns is correct. It is still exactly the defect `strnlen` exists to
prevent: on a field of four characters in four bytes it walks past the end of the object
looking for a terminator that was never there. The harness catches it in the ASan stage, not
the plain one, with `READ of size 1` and `my_strlen` in frame `#0` — a reminder that a
function can be "tested and correct" on every output and still be wrong about what it is
allowed to touch.

Second: `while (s[n] != '\0' && n < maxlen)`, the right conditions in the wrong order. Same
numbers, same over-read, and harder to see because the bound *is* there.

Third: returning -1 or `SIZE_MAX` when no terminator is found, which feels more explicit. It
is a different function from `strnlen`, it makes every caller test for a magic value, and it
throws away the useful half of the answer: how many bytes there were.

## How libc does it

glibc's `strlen` reads an aligned machine word at a time and tests all of its bytes for zero
with a bit trick (`(w - 0x0101...) & ~w & 0x8080...`), or a whole vector register at a time
with one compare instruction, then finds which byte matched. The loop therefore runs once per
8 to 64 bytes. It is allowed to read bytes past the terminator inside the aligned block it has
already touched, because such a block cannot cross a page boundary — a trade a libc can make,
knowing its platform, and a portable C program cannot.

`strnlen` gets the same treatment through `memchr`, with one property worth knowing: because
it must not read past `maxlen`, the vectorised version has to finish the tail byte by byte,
which can make `strnlen(s, 4)` marginally *slower* than `strlen(s)` on a very short string.
That is never a reason to choose `strlen` for a field whose terminator you are not sure of.
