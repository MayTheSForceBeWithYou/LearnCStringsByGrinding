# s03-e01-my-strcpy-stpcpy: notes

## Why it is correct

`while ((*dst = *src) != '\0')` copies one byte and then tests the byte it copied. The
terminator is therefore copied before the loop stops, and nothing after it is read or
written. When the loop ends, `dst` points at the terminator just written, which is exactly
what `stpcpy` returns. `my_strcpy` is the same copy with the original pointer returned.

Writing `stpcpy` first and `strcpy` on top of it is the natural order: the end pointer is
what the loop has in hand, and the start pointer is what the caller already had.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| empty source | the first assignment copies `'\0'`; the body never runs; returns `dst` |
| exact-size destination | exactly `strlen(src) + 1` assignments, no extra write |
| bytes above 0x7F | the test is `!= '\0'`, not `> 0`; signedness of `char` never matters |
| bytes after the terminator | never touched: the loop stops at the first NUL |

## Alternatives

```c
char *d = dst;
while (*src != '\0')
    *d++ = *src++;
*d = '\0';
```

The same thing in three statements; the terminator is written separately, and forgetting
that last line is the usual mistake below. With a known length, `memcpy(dst, src, len + 1)`
is the better tool, and it is what the exercise forbids.

## The usual mistake

Stopping the loop *before* copying the terminator (`while (*src) *dst++ = *src++;`) and not
adding it afterwards. Tests that copy into a zeroed buffer pass. The harness pre-fills the
destination with `#`, so the missing terminator shows as `hello#`.

Second: returning `dst` from `my_stpcpy` after the loop has advanced it "one too far"
(`*dst++ = *src++` in the condition leaves `dst` one past the terminator).

## How libc does it

Not a byte at a time. glibc and Apple's libc find the source's length with a vectorised
scan (16 to 64 bytes per step, using SSE2/AVX2 or NEON to test a whole block for a zero
byte) and then copy with the `memcpy` machinery. The portable C fallback reads a machine
word at a time and uses a bit trick to detect a zero byte inside it. Both rely on reading a
few bytes past the terminator within the same aligned word or page, which a C program is
not allowed to do and a libc, which knows its platform, is.
