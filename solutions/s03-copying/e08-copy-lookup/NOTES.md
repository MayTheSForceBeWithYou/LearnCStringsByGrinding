# s03-e08-copy-lookup: notes

## Why each answer is what it is

`ANSWER.md` in this directory has the supporting sentences, quoted from Linux man-pages
6.19. macOS words them differently and says the same things.

| Key | Answer | In one line |
|---|---|---|
| `strcpy.returns` | `dst` | the pointer you passed in |
| `stpcpy.returns` | `end` | a pointer to the terminating NUL of the copy |
| `strncpy.pads` | `yes` | a short source is followed by NULs up to `n` |
| `strncpy.terminates_full` | `no` | six characters, six bytes, no NUL |
| `stpncpy.full_offset` | `6` | `dst + strnlen(src, n)`, which is `dst + n` here |
| `strcpy.header` | `string.h` | SYNOPSIS |
| `strcpy.overlap_allowed` | `no` | CAVEATS; also what `restrict` in the prototype means |
| `stpcpy.iso_c17` | `no` | STANDARDS lists POSIX.1-2008 only |
| `strncpy.reports_truncation` | `no` | it returns `dst` whatever happened |

## Where the answers live

Return values are in RETURN VALUE and nowhere else reliable. Standards conformance is in
STANDARDS (CONFORMING TO on older Linux pages). The warnings are in CAVEATS or BUGS, near the
end, which is why they are missed: most readers stop after DESCRIPTION.

## Alternatives

`man 7 string_copying` (Linux) answers most of these on one page and is worth reading once
in full. The C standard itself (C17 7.24.2.3 and 7.24.2.4) is the authority for `strcpy` and
`strncpy`; POSIX.1-2008 (`man 3p stpcpy` where the POSIX pages are installed) for the other
two.

## The usual mistake

Answering from memory, getting `strncpy.terminates_full` right and `stpcpy.iso_c17` wrong.
`stpcpy` is so ordinary on Linux and macOS that it feels standard. It is POSIX; a strict ISO
C17 build (`-std=c17` with no feature-test macro on glibc) does not declare it.

## How libc does it

Not applicable. One fact about the pages instead: the Linux `strncpy(3)` page was rewritten
in 2023 around the phrase "null-padded character sequence, not a string". If your page still
describes `strncpy` as copying "at most n bytes" of a string with a warning underneath, it
is the older text; the function is the same.
