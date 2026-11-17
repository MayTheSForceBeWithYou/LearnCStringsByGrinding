# s01-e03-empty-wraps: answer

## The defect

`stats.c`, line 11, in `count_upper`:

```c
    for (size_t i = 0; i <= strlen(s) - 1; i++) {
```

`count_digits`, four lines below it, is the same loop written correctly:
`for (size_t i = 0; s[i] != '\0'; i++)`. It needs no length, so it has no subtraction to get
wrong.

## The mechanism

`strlen("")` is 0: the empty string is a terminator and nothing else, so there are no bytes
before it.

`0 - 1` in `size_t` is not -1. `size_t` is an unsigned type, and C17 6.2.5p9 says that a
result which cannot be represented in an unsigned type is reduced modulo 2^N, where N is the
number of value bits. On both course platforms `size_t` is 64 bits, so the result is
2^64 - 1 = 18446744073709551615, which is `SIZE_MAX`. The subtraction did not overflow in any
detectable sense; wrapping is what the type is defined to do.

The loop condition is therefore `i <= SIZE_MAX`, which is true for every value `i` can hold.
There is no `size_t` larger than `SIZE_MAX`, so the loop has no exit. It reads `s[0]` — the
terminator, which is inside the object and fine — and then `s[1]`, which is the first byte
after a one-byte buffer and the first byte it was not allowed to touch. From there it walks
forward one byte at a time until the process leaves memory the kernel has mapped for it.

The condition also calls `strlen(s)` on every iteration, so even for a non-empty string the
loop is quadratic in the string's length. That is not what crashes it, but it is the second
reason the shape is wrong.

## How the tool showed it

The error kind is `heap-buffer-overflow`: the address belongs just outside a heap block ASan
knows about, so it can say which block and how big it was. `READ of size 1` says the program
was loading one byte, not storing — so this is a reader that ran off the end, never a copy
that did not fit. `0 bytes after 1-byte region` is the decisive line: the region is one byte,
which can only be the empty string's terminator, and the bad byte is the one immediately
after it. Frame `#0` is `stats.c:11`, the loop itself: both the access and the defect are on
the same line, which is not usually the case in this course.

The plain build prints no `FAIL` line for the empty-string case because `count_upper` never
returns, so the `check` that would have compared its result never runs. The process is killed
by `SIGSEGV` once it reads past the end of the heap mapping, and `make` reports exit status
139, which is 128 + 11: killed by signal 11. The seven earlier cases had already passed
silently, so the whole output of the plain stage is one `make` error line.

## The rule you take away

Never subtract from an unsigned length without first knowing it is not zero: write the loop
that stops at the terminator instead of the loop that counts to `length - 1`.
