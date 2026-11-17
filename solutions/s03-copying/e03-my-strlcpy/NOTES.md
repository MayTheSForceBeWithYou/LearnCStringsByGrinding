# s03-e03-my-strlcpy: notes

## Why it is correct

One loop walks the whole source, so `i` ends as `strlen(src)`, which is the return value.
Inside the loop a character is stored only while `i + 1 < size`, that is, while storing it
still leaves a byte for the terminator. After the loop, if `size > 0`, the terminator goes at
`i` when the string fitted and at `size - 1` when it did not.

`i + 1 < size` is written that way rather than `i < size - 1` because `size - 1` wraps to
`SIZE_MAX` when `size` is 0, and the loop would then write into a buffer with no room.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `size == 0` | `i + 1 < 0` is never true, so no character is stored; `if (size > 0)` skips the terminator |
| `size == 1` | `i + 1 < 1` is never true; the terminator goes at index 0 |
| exact fit (`len == size - 1`) | all characters stored; terminator at `i == size - 1` |
| one too many (`len == size`) | the last character is not stored; terminator at `size - 1`; returns `size` |
| long source | the loop keeps counting after the buffer is full |

## Alternatives

```c
size_t len = strlen(src);
if (size > 0) {
    size_t n = len < size ? len : size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}
return len;
```

Two passes over the part that fits, but `memcpy` is so much faster per byte than a character
loop that this is what the BSD and glibc implementations do. In portable C17 with no
`strlcpy`, `snprintf(dst, size, "%s", src)` is the library call with the same contract.

## The usual mistake

Stopping the loop when the buffer is full and returning the number of characters copied.
The return value is then `size - 1` for every truncated copy, the caller's `ret >= size`
test is never true, and truncation has become silent again, which is the defect the
function was invented to remove.

Second: `dst[size - 1] = '\0'` with `size == 0`.

## How libc does it

OpenBSD's original is a single loop much like this one. glibc 2.38 and Apple use
`strlen` followed by `memcpy`. All of them read the entire source; that cost is part of the
contract, and it is the main criticism of `strlcpy` (it must not be given an unterminated
source, however small `size` is).
