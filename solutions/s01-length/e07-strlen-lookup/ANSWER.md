# s01-e07-strlen-lookup: the sentences

Quoted from Linux man-pages 6.19.

## strlen.empty

Page and section: strlen(3), DESCRIPTION

> The strlen() function calculates the length of the string pointed to by s, excluding the
> terminating null byte ('\0').

Applied to the empty string: it is a terminator and nothing else, so the number of bytes
excluding it is `0`. The page gives no separate rule for the empty string and does not need
one.

## strnlen.zero

Page and section: strnlen(3), DESCRIPTION

> This function determines the length of the character sequence contained in the array
> pointed to by strn.

and the identity the same page gives:

> It is equivalent to
>
>     (memchr(strn, '\0', n) ?: strn + n) - strn

With `n == 0`, `memchr` examines no bytes and returns NULL, so the expression is
`strn + 0 - strn`, which is `0`. No byte of the array is read.

## strnlen.full_field

Page and section: strnlen(3), DESCRIPTION and RETURN VALUE

> If strn contains a string, the array can have less than n elements.

> The strnlen() function returns the number of nonnull characters in strn.

For four characters in four bytes with no terminator, every one of the `n` bytes is a nonnull
character, so the answer is `4`. The same identity gives it directly: `memchr` finds no zero
byte in the four, so the result is `strn + n - strn`, which is `n`.

## strlen.in_iso_c

Page and section: strlen(3), STANDARDS

> C11, POSIX.1-2008.

`yes`: `strlen` is in ISO C (C89 and every edition since; the page names the edition it was
checked against).

## strnlen.in_iso_c

Page and section: strnlen(3), STANDARDS

> POSIX.1-2008.

`no`: ISO C is not listed. HISTORY on the same page says "POSIX.1-2008", so the function has
never been in ISO C. That is why it needs a feature-test macro and why nothing in the ISO C
standard library can be relied on to measure a field that may not be terminated.

## strnlen.macro

Page and section: strnlen(3), SYNOPSIS, Feature Test Macro Requirements block

> Feature Test Macro Requirements for glibc (see feature_test_macros(7)):
>
>     strnlen():
>         Since glibc 2.10:
>             _POSIX_C_SOURCE >= 200809L
>         Before glibc 2.10:
>             _GNU_SOURCE

So the macro is `_POSIX_C_SOURCE`.

## strnlen.macro_value

Page and section: the same block

`200809L`. The page writes the requirement as an inequality (`>= 200809L`), so a larger value
also works; 200809L is the value this course's build defines, in `lib/dsalloc.h`.

macOS note: there is no Feature Test Macro Requirements block on that platform's pages, and
`strnlen` is declared unconditionally.

## strnlen.header

Page and section: strnlen(3), SYNOPSIS

> #include <string.h>  // see memory.h(3head)

`string.h`. (The comment is a man-pages convention pointing at a page that discusses which
header really declares the memory and string functions; the answer is still `string.h`.)

## strlen.return_type

Page and section: strlen(3), SYNOPSIS

> size_t strlen(const char *s);

`size_t`: unsigned, wide enough for the size of any object. That it is unsigned is the whole
content of e03.

## Two questions with no key

1. **The words the two pages use.** strlen(3) says "the length of the **string** pointed to by
   s"; strnlen(3) says "the length of the **character sequence** contained in the array pointed
   to by strn". `string_copying(7)`, Terms, defines both:

   > string (str) is a sequence of zero or more non-null characters followed by a null
   > character.

   > character sequence is a sequence of zero or more non-null characters. A program should
   > never use a character sequence where a string is required.

   So for a fixed-width database field, which may be full and therefore have no terminator,
   the function to use is `strnlen`: it is the one documented to take a character sequence.
   Even the parameter names say it — `s` against `strn`.

2. **Why `maxlen` rather than an error value.** `strnlen` returns `size_t`, which is unsigned,
   so there is no negative value available and -1 would be `SIZE_MAX` — a legal length in
   principle, and a value a caller would have to remember to test for. Returning `maxlen`
   keeps every result a true answer to "how many bytes of text are there in the first `maxlen`
   bytes", makes the "no terminator" case detectable with the comparison the caller already
   has (`ret == maxlen`), and composes: `memcpy(dst, field, strnlen(field, width))` is correct
   whether the field is full or padded, with no branch at all.
