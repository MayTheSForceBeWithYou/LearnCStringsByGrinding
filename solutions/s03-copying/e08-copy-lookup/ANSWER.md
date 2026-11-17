# s03-e08-copy-lookup: the sentences

Quoted from Linux man-pages 6.19.

## strcpy.returns

Page and section: strcpy(3), RETURN VALUE

> These functions return dst.

## stpcpy.returns

Page and section: stpcpy(3), RETURN VALUE

> This function returns a pointer to the terminating null byte of the copied string.

## strncpy.pads

Page and section: strncpy(3), DESCRIPTION

> If the source has too few non-null bytes to fill the destination, it pads the destination
> with trailing null bytes.

## strncpy.terminates_full

Page and section: strncpy(3), CAVEATS

> This function produces a null-padded character sequence, not a string (see
> string_copying(7)).

and the example below it:

> strncpy(buf, "12345", 5);   // { '1', '2', '3', '4', '5' }

## stpncpy.full_offset

Page and section: stpncpy(3), RETURN VALUE

> This function returns a pointer to one past the last non-null character written, that is,
> dest + strnlen(src, n).

For `stpncpy(buf, "abcdef", 6)`, `strnlen("abcdef", 6)` is 6.

## strcpy.header

Page and section: strcpy(3), SYNOPSIS

> #include <string.h>

## strcpy.overlap_allowed

Page and section: strcpy(3), CAVEATS

> The strings src and dst may not overlap.

## stpcpy.iso_c17

Page and section: stpcpy(3), STANDARDS

> POSIX.1-2008.

(Compare strcpy(3), STANDARDS: "C11, POSIX.1-2008.")

## strncpy.reports_truncation

Page and section: strncpy(3), RETURN VALUE and CAVEATS

> dst.

> It's impossible to distinguish truncation by the result of the call, from a character
> sequence that just fits the destination buffer; truncation should be detected by comparing
> the length of the input string with the size of the destination buffer.

## One more, not graded

strncpy(3), EXAMPLES:

> This function is intended to be used only for writing to fixed-width null-padded character
> arrays, such as those in utmp(5) members.
