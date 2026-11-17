# s00-e07-string-lookup: notes

## Why each answer is what it is

| Key | Answer | Section that settles it |
|---|---|---|
| `strlen.counts_terminator` | `no` | DESCRIPTION: "excluding the terminating null byte" |
| `strlen.empty` | `0` | the same sentence, applied: the empty string is only a terminator |
| `strlen.header` | `string.h` | SYNOPSIS |
| `strlen.return_type` | `size_t` | SYNOPSIS |
| `strlen.param_const` | `yes` | SYNOPSIS: `const char *s` |
| `strlen.unterminated` | `undefined` | nothing in the page: the parameter is documented as a string |
| `strlen.bounded_name` | `strnlen` | `man -k "string length"`, then strnlen(3) |
| `puts.appends_newline` | `yes` | DESCRIPTION: "writes the string s and a trailing newline" |
| `puts.error_return` | `EOF` | RETURN VALUE |

## Numbers worth noticing

Two of the nine answers are about what the documentation leaves out, and they are the two
worth most. `strlen.unterminated` is undefined because the page describes the function's
behaviour on a *string*, and a reader who wants to know about other bytes has to notice that
the page never mentions them. `puts.error_return` is `EOF` and the success value is only
"a nonnegative number" — so the page is explicitly refusing to promise what the success value
means, which tells you not to build anything on it.

`size_t` for the return type is the third. It is unsigned, and that is the source of the
`for (i = 0; i <= strlen(s) - 1; i++)` disaster on an empty string, which is S01 e03: 0 - 1 in
`size_t` is `SIZE_MAX`, and the loop reads the whole address space.

## Alternatives

Three other ways to answer the same questions, in the order a working programmer reaches for
them:

| Tool | Good for |
|---|---|
| `man 3 <name>` | the contract: prototype, return value, standard, caveats |
| `man -k <words>` / `apropos` | finding the page when you half-remember the name |
| the header itself (`/usr/include/string.h`) | what *this* libc declares, under which feature macros |
| the standard (C17 7.24, or n3096 for C23) | what every implementation must do, which is less than glibc does |

The header is the authority on whether a function is declared in your build; the page is the
authority on what it does; the standard is the authority on what you may rely on. The three
disagree often enough to be worth keeping apart, and `strnlen` is an example: declared in
`<string.h>` here only because the course defines `_POSIX_C_SOURCE 200809L`, documented on its
own page, and absent from ISO C entirely.

## The usual mistake

Answering from memory and checking with the grader. The grader says `WRONG` without saying
why, which is deliberate: there is nothing to learn from a wrong verdict on a fact, and
everything to learn from the sentence that settles it. The second mistake is quoting a
paraphrase in `ANSWER.md` ("the page says it returns the length") rather than the words, which
is how a reader convinces themself that a page says what they already believed.

A third, specific to this exercise: answering `a number` for `strlen.unterminated`, because
the call does return a number when you try it. It does on this machine, this time, with this
compiler. "Undefined" is a statement about the standard's requirements, not a prediction about
one run, and a program whose correctness depends on which number came out is already broken.

## How libc does it

The manual pages are not the library; they are a separate project (`man-pages` on Linux,
Apple's own documentation on macOS) that documents it, and they occasionally describe their
own libc rather than the standard. Linux's `strlen(3)` says "It is equivalent to
`strnul(s) - s`", which mentions a glibc-specific function the standard has never heard of;
the STANDARDS section is where the page tells you what is portable (`C11, POSIX.1-2008` for
`strlen`).

That split is why this course keeps citing clause numbers (`C17 7.24.6.3` for `strlen`) next
to page sections. The page is the fastest way to the answer; the standard is the answer when
two platforms disagree. S15 has the cases where they genuinely do.
