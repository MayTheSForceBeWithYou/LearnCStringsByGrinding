# s01-e07-strlen-lookup: notes

## Why each answer is what it is

| Key | Answer | Section that settles it |
|---|---|---|
| `strlen.empty` | `0` | DESCRIPTION: "excluding the terminating null byte" |
| `strnlen.zero` | `0` | DESCRIPTION, and the `memchr` identity with `n == 0` |
| `strnlen.full_field` | `4` | RETURN VALUE: "the number of nonnull characters in strn" |
| `strlen.in_iso_c` | `yes` | STANDARDS: `C11, POSIX.1-2008` |
| `strnlen.in_iso_c` | `no` | STANDARDS: `POSIX.1-2008` only |
| `strnlen.macro` | `_POSIX_C_SOURCE` | the Feature Test Macro Requirements block |
| `strnlen.macro_value` | `200809L` | the same block |
| `strnlen.header` | `string.h` | SYNOPSIS |
| `strlen.return_type` | `size_t` | SYNOPSIS |

## Numbers worth noticing

The two STANDARDS lines are the most consequential pair of answers in the unit. `strlen` is
ISO C, so it is available in every conforming implementation, with no macros and no
conditions — and it is the function that cannot be used safely on a field that may be full.
`strnlen` is the one you want, and it is POSIX only, which means a strictly portable ISO C
program does not have it at all. Such a program has to use `memchr` (ISO C, bounded) and do
the subtraction itself, which is exactly the identity the Linux `strnlen` page prints.

So the hierarchy, for a length over bytes that may not be terminated:

| Available where | Call |
|---|---|
| ISO C, everywhere | `memchr(buf, '\0', n)` and subtract, or `n` if NULL |
| POSIX.1-2008 and both course platforms | `strnlen(buf, n)` |
| nowhere portable | anything that calls `strlen` on it |

## Alternatives

Three sources, and what each one is the authority on:

| Source | Authority on |
|---|---|
| the manual page | what this implementation does, and which standard it claims |
| `/usr/include/string.h` | whether the function is *declared* in your translation unit, and under which macros |
| the standard itself (C17 7.24, POSIX.1-2008) | what you may rely on across implementations |

They disagree often enough to be worth keeping apart. For `strnlen` all three are needed to
answer "may I call this?": the standard says it is POSIX, the header says it needs
`_POSIX_C_SOURCE >= 200809L`, and the page tells you both. Reading the header is a habit worth
having — `grep -n strnlen /usr/include/string.h` shows the `#if` that decides it.

## The usual mistake

Answering `strnlen.full_field` with `undefined`, by analogy with `strlen`. The whole point of
`strnlen` is that it has a defined answer there: the bound makes the call legal, and the
answer is the bound. Getting this one wrong usually means the reader skipped the parameter
names: `strlen(const char *s)` against `strnlen(const char *strn, size_t n)`, where even the
name says "not necessarily a string".

The second is answering the macro question from memory as `_GNU_SOURCE`. That works, and the
page says so for glibc before 2.10 — but it is the wrong answer for a portable program,
because `_GNU_SOURCE` turns on every GNU extension and this course forbids it (`docs/SPEC.md`
section 10). `_POSIX_C_SOURCE 200809L` asks for exactly one standard's worth of functions,
which is why it is the macro the course's build defines.

The third is quoting a paraphrase in `ANSWER.md`. "The page says strnlen is POSIX" is not
evidence; `POSIX.1-2008.` under the heading `STANDARDS` is.

## How libc does it

glibc implements `strnlen` as a thin wrapper over its vectorised `memchr` on most
architectures, which is why the manual page gives the `memchr` identity as the function's
definition rather than as an analogy. Apple's libc does the same.

The feature-test machinery is worth one more sentence, because it is the reason a build can
fail in a way that looks like a missing library. Defining `_POSIX_C_SOURCE` before including
anything changes which declarations the headers expose, not which symbols the library
contains: `strnlen` is in `libc.so` either way. Forget the macro and the compiler sees an
implicit declaration — which C17 removed, so it is now an error — while the linker would have
found the symbol perfectly well. The fix is always at the top of the translation unit, never
at the link line, and in this course `lib/dsalloc.h` has already done it.
