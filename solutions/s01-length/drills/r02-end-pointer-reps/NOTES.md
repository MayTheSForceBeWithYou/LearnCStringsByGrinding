# s01-r02-end-pointer-reps: notes

## Why it is correct

`end_of` walks to the terminator and returns the pointer the loop is holding, which is by
definition the address of that terminator. It is `strlen` with the subtraction left out — and
for an appender, the pointer is the useful half, because it is where the next write goes.

`last_char` asks the empty question first and then reuses `end_of`:

```c
char *e = end_of(s);
return e == s ? NULL : e - 1;
```

`e == s` is true exactly when the string is empty, so the NULL case is settled without a
second walk, and `e - 1` is inside the object whenever it is not. Writing it as
`end_of(s) - 1` with no test would hand back a pointer one *before* the object for the empty
string, which is undefined pointer arithmetic even if nothing dereferences it.

`len_between` is the subtraction, with the conversion written down:

```c
return (size_t)(end - begin);
```

The difference of two pointers into one object has type `ptrdiff_t` (signed) and is defined
only when both point into the same object, one past the end included (C17 6.5.6p9). The
function's contract carries that requirement, which is why it can be one line.

`room_left` is the arithmetic an appender does before every write:

```c
size_t bytes = len_between(p, buf_end);
return bytes > 0 ? bytes - 1 : 0;
```

`bytes` is how many bytes are available at `p`; one of them belongs to the terminator, so the
number of *characters* is one fewer. The `bytes > 0` test is what keeps `bytes - 1` from
wrapping to `SIZE_MAX` when `p` has already reached `buf_end` — the same unsigned subtraction
that e03 is about, in the one place in this drill where it can happen.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | `end_of` returns `s`; `last_char`'s `e == s` test returns NULL |
| a one-character string | `end_of` is `s + 1`, `last_char` is `s` |
| `len_between` of one point | `end - begin` is 0 |
| `p` at the buffer's last byte | `bytes` is 1, so `room_left` is 0: only the terminator fits |
| `p == buf_end` | `bytes` is 0, and the `> 0` test returns 0 instead of wrapping |
| a full buffer | `room_left` reaches 0 and the writing loop stops |
| bytes above 0x7F | nothing here compares a byte's value except against `'\0'` |

## Alternatives

| This function | The library, or the idiom |
|---|---|
| `end_of` | `s + strlen(s)`; or `stpcpy`'s return value, which gives it away free while copying (S03) |
| `last_char` | `s[strlen(s) - 1]` **after** testing for the empty string; or `strrchr` for a particular byte (S08) |
| `len_between` | the subtraction itself; `%.*s` with `(int)(end - begin)` prints such a range (S11) |
| `room_left` | nothing in the library: `snprintf`'s `cap` parameter plays the same role, one call at a time |

The pair `(p, buf_end)` against the pair `(buf, cap)` is a real design choice, not a style
one. Lengths travel better through interfaces — every library function takes a size — while
end pointers compose better inside one function, because advancing `p` is the only
bookkeeping and nothing has to be recomputed. The Linux manual pages give the pointer form a
name: `string_copying(7)` calls a function that takes the end "chain"-style, and its
`stpecpy` sketch is exactly `room_left` plus a copy.

Where end pointers genuinely win is a loop that appends many pieces: with lengths, each
append needs `cap - used` computed from a `used` that has to be maintained; with pointers,
`p` *is* the state. S04 e05 builds that cursor.

## The usual mistake

Forgetting the terminator in `room_left`, so that it returns `buf_end - p`. Everything then
works until the buffer is exactly full, at which point the terminator is written at
`buf[cap]`, one byte past the end — a fence failure in the plain build and a
`heap-buffer-overflow WRITE of size 1` under ASan. The harness checks `room_left` at the
buffer's last byte and at `buf_end` precisely to catch this.

Second: `last_char` written as `end_of(s) - 1` with no empty test. For `""` that is `s - 1`,
which is undefined even unevaluated, and the function's "NULL if empty" contract is gone.

Third: computing `room_left` by walking the string (`strlen` of what is there) instead of
subtracting pointers. It gives the same number while the buffer holds a string, and it is a
pass over the data on every append — the quadratic shape of S04 e03, built from a function
whose whole purpose was to avoid it.

## How libc does it

The C library has no end-pointer interface: every function takes a pointer and a size, or a
pointer and a terminator. The closest thing is `stpcpy` and `stpncpy`, which *return* the end
of what they wrote so that the caller can keep chaining, and `memccpy`, which returns the byte
after the stop byte. The Linux `string_copying(7)` page is the one place the pointer style is
written down as a family, including functions (`stpecpy`, `strtcpy`) that are proposals and
examples rather than anything you can call.

So the pair in this drill is the shape you build for yourself, over the library's
size-taking functions: measure or write with `snprintf`/`memcpy`, advance `p`, ask
`room_left` again. That is what S04's cursor is, and keeping the arithmetic in two tiny
functions is what stops it from being wrong once per call site.
