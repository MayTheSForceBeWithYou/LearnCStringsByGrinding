# s04-e08-strcat-lookup: the sentences

Quoted from Linux man-pages 6.19.

## strcat.returns

Page and section: strcat(3), RETURN VALUE

> These functions return dst.

`dst` — the pointer the caller already had. The function cannot fail and has nothing else to
report.

## strncat.result

Page and section: strncat(3), DESCRIPTION

> This function appends at most ssize non-null bytes from the array pointed to by src,
> followed by a null character, to the end of the string pointed to by dst.

With `dst` holding `"ab"`, `src` `"cdef"` and `ssize` 2: two non-null bytes, `c` and `d`, then
a null character. So `abcd`.

## strncat.bytes_written

Page and section: strncat(3), DESCRIPTION, the same sentence

Two characters plus the terminator: `3` bytes. The general form is
`min(ssize, strlen(src)) + 1`, and the `+ 1` is the "followed by a null character" of the
quoted sentence — it is written unconditionally, which is what makes `strncat` write up to
`n + 1` bytes.

## strncat.short_src

Page and section: strncat(3), DESCRIPTION, the same sentence

"At most ssize" — the source has only two characters, so both are appended and the terminator
follows: `abcd`. A larger `ssize` does not make the function read further; it stops at the
source's own terminator.

## strcat.need

Page and section: strcat(3), DESCRIPTION

> The programmer is responsible for allocating a destination buffer large enough, that is,
> strlen(dst) + strlen(src) + 1.

With 3 and 5: 3 + 5 + 1 = `9`.

## strncat.need

Page and section: strncat(3), DESCRIPTION

> dst must point to a string contained in a buffer that is large enough, that is, the buffer
> size must be at least strlen(dst) + strnlen(src, ssize) + 1.

With `strlen(dst)` 3, a source of 5 characters and `ssize` 2: `strnlen(src, 2)` is 2, so
3 + 2 + 1 = `6`.

Note what the formula is **not**: it is not `cap`, and it does not involve the destination's
size at all. It tells the caller how large the buffer must be; the caller still has to work
out the third argument from the size it has.

## strncat.max_bytes

Page and section: strncat(3), DESCRIPTION

At most `ssize` non-null bytes plus one null character, so with `ssize` 4 the answer is `5`.

## strncat.reports_truncation

Page and section: strncat(3), RETURN VALUE

> strncat() returns dst.

`no`. The return value is the same whether the source was appended whole, clipped by `ssize`,
or empty. A caller who needs to know must compare the lengths itself — which is why `strlcat`
and `snprintf` exist.

## strlcat.in_posix_2008

Page and section: strlcat(3), STANDARDS

> None.

and HISTORY:

> OpenBSD 2.4, FreeBSD 3.3, glibc 2.38.

`no`. The function is a BSD extension that reached glibc only in 2.38 and POSIX only in
POSIX.1-2024 — after the standards this course compiles against, which is why `lib/dsalloc.h`'s
`_POSIX_C_SOURCE 200809L` does not declare it and why e02 asks you to write it.

macOS note: the page there is `strlcpy(3)`, shared with `strlcat`, and it describes them as
BSD functions without a STANDARDS section of this shape.

## Two questions with no key

1. **The `strcat` equivalence.** strcat(3), DESCRIPTION:

   > It is equivalent to
   >
   >     stpcpy(strnul(dst), src), dst

   `strnul(dst)` is glibc's "pointer to the terminator" — the `end_of` of S01 drill r02 — so
   the line says: walk `dst` to its end, chain-copy `src` from there, return `dst`. The walk
   is the cost: `strcat` is `O(strlen(dst) + strlen(src))`, and in a loop that appends `n`
   pieces the walks add up to `O(total²)`. That is e03, and the equivalence is the clearest
   statement of why no implementation can fix it: the function is not told where the end is.

2. **The correct third argument.** To append safely into a buffer of `cap` bytes that already
   holds a string:

   ```c
   strncat(dst, src, cap - strlen(dst) - 1);
   ```

   `cap` is the buffer's total size; `strlen(dst)` is what is already there, so `cap -
   strlen(dst)` is the bytes left starting at the terminator; the `- 1` reserves the byte for
   the terminator `strncat` always writes. With `cap == 0` the expression must not be
   evaluated at all — it would wrap — so the call needs a guard. That is e04.
