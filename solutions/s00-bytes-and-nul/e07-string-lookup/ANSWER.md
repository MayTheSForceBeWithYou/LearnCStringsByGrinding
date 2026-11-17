# s00-e07-string-lookup: the sentences

Quoted from: Linux man-pages 6.19.

## strlen.counts_terminator

Page and section: strlen(3), DESCRIPTION

> The strlen() function calculates the length of the string pointed to by s, excluding the
> terminating null byte ('\0').

So the answer is `no`: the terminator is excluded. RETURN VALUE says the same thing from the
other side ("returns the number of bytes in the string pointed to by s"), which is why the
length of a string and the storage it needs differ by exactly one.

## strlen.empty

Page and section: strlen(3), DESCRIPTION, applied

The page gives no separate rule for the empty string, and it does not need one: the empty
string is a terminator and nothing else, so the number of bytes "excluding the terminating
null byte" is `0`. Worth noticing that the page's definition covers the edge case without
mentioning it.

## strlen.header

Page and section: strlen(3), SYNOPSIS

> #include <string.h>

## strlen.return_type

Page and section: strlen(3), SYNOPSIS

> size_t strlen(const char *s);

`size_t`: unsigned, wide enough for the size of any object, and the type every length and
capacity in this course is kept in.

## strlen.param_const

Page and section: strlen(3), SYNOPSIS, the same line

`const char *s`, so the answer is `yes`. The `const` is the function's promise not to write
through the pointer, which is why `strlen` may be called on a string literal.

## strlen.unterminated

The sentence that says what `strlen` requires of its argument, strlen(3), DESCRIPTION:

> The strlen() function calculates the length of the string pointed to by s, excluding the
> terminating null byte ('\0').

and string_copying(7), DESCRIPTION, Terms:

> string (str) is a sequence of zero or more non-null characters followed by a null character.

> character sequence is a sequence of zero or more non-null characters. A program should
> never use a character sequence where a string is required.

What the page does not say: there is no sentence describing what `strlen` returns when the
bytes contain no null character, because there is nothing to describe. The parameter is
documented as a *string*, a string is defined as ending in a null character, and passing
something else breaks the function's requirement. The answer is `undefined`: the C standard
imposes no requirement at all (C17 4p2), so the call may return any number, crash, or be
reported by a sanitizer, and all three are conforming behaviour.

macOS note: there is no `string_copying(7)` page there; `strlen(3)` alone carries the
argument's description, and the reasoning is the same.

## strlen.bounded_name

Page and section: strnlen(3), SYNOPSIS (found with `man -k "string length"`, which lists
`strlen (3) - string length` and `strnlen (3) - nonstring length`)

> size_t strnlen(const char strn[], size_t n);

`strnlen` is the answer. Its own DESCRIPTION is the clearest statement in the manual of what
the two functions are for:

> This function determines the length of the character sequence contained in the array
> pointed to by strn.

A *character sequence*, not a string: `strnlen` is the function you use on bytes that may not
be terminated, which is why S01 e02 is built on it. On macOS it shares the `strlen(3)` page.

## puts.appends_newline

Page and section: puts(3), DESCRIPTION

> puts() writes the string s and a trailing newline to stdout.

`yes`. The same page's line about `fputs` is the contrast worth remembering:

> fputs() writes the string s to stream, without its terminating null byte ('\0').

`fputs` writes exactly the characters; `puts` adds a newline. Neither writes the terminator.

## puts.error_return

Page and section: puts(3), RETURN VALUE

> puts() and fputs() return a nonnegative number on success, or EOF on error.

`EOF`. Note what it does *not* promise: not "the number of characters written", not 0, just
some nonnegative number. Code that tests `puts(s) == 0` is testing something the page never
agreed to.

## Two questions with no key

1. **Length versus size.** `string_copying(7)`, Terms, defines both:

   > length (len) is the number of non-null characters in a string or character sequence. It
   > is the return value of strlen(str) and of strnlen(buf, size).

   > size refers to the entire buffer where the string or character sequence is contained.

   A buffer has a size; the text in it has a length. A string of length `len` needs `len + 1`
   bytes of size, so it fits when `len < size`. Every parameter named `cap` or `size` in this
   course includes the terminator; every one named `len` or `n` does not.

2. **Why `puts` returns an `int`.** Writing to standard output can fail: the disk is full, the
   pipe has been closed (which also raises `SIGPIPE`), the device is gone, or a buffered write
   fails later at `fflush` or `fclose`. A program whose output matters checks it — at least
   once, at the end, with `fflush(stdout)` and `ferror(stdout)` — because a program that
   prints a report and exits 0 after the disk filled up has lied to whatever ran it. S12 is
   where `ferror` and `feof` get their cards.
