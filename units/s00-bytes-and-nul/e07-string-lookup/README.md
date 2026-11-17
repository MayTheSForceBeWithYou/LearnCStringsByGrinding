# s00-e07-string-lookup `[LOOKUP]`

## Goal

Answer nine questions about `strlen` and `puts` **from the manual pages**, and quote the
sentence that supports each answer. Afterwards you know the shape of a section-3 manual page,
you have read the library's own definition of the word "string", and you have seen in print
the one thing `strlen` does not promise.

## Background

Every function this course covers is documented on the machine you are reading this on, and
the documentation is the primary source: not the lesson, not a web search, not what you
remember from another language. Being able to answer a question about a library function from
its page in under a minute is a skill, it is trained by doing it, and this is the first of
twenty times you will do it here.

A section-3 page has a fixed shape, and each kind of question has a home:

| You want to know | Look in |
|---|---|
| which header to include, the exact prototype, `const` and `restrict` | **SYNOPSIS** |
| what the function does to its arguments | **DESCRIPTION** |
| what comes back, and how an error is signalled | **RETURN VALUE** (macOS: **RETURN VALUES**) |
| which standard defines it | **STANDARDS** (older Linux pages: **CONFORMING TO**) |
| the traps | **CAVEATS**, **BUGS**, **NOTES** (macOS: **SECURITY CONSIDERATIONS**) |
| the neighbours worth knowing about | **SEE ALSO** |

The pages you need here:

| | Linux (man-pages) | macOS |
|---|---|---|
| `strlen` | `man 3 strlen` | `man 3 strlen` |
| `puts` | `man 3 puts` (shared with `fputs`, `putc`, `putchar`) | `man 3 puts` (shared with `fputs`) |
| the family index | `man 3 string` | `man 3 string` |
| the vocabulary | `man 7 string_copying` (**Terms**) | not available |

`string_copying(7)` is a Linux-only page and it is worth ten minutes of your life: its
**Terms** section is where the man-pages define *string*, *character sequence*, *length* and
*size* — the four words this whole course turns on — and it says in one line why a buffer
without a terminator may not be handed to a function that wants a string. If you are on
macOS, you cannot read that page; answer the questions from `strlen(3)` and `puts(3)`, which
are enough, and quote those.

One question is about something the page does **not** say, and that is deliberate. The page
tells you what `strlen` does with a string. It is silent about what happens when the bytes
are not one, and the silence is the answer: the standard imposes no requirement, which is
what "undefined" means. Learning to notice an absence in documentation is as useful as
reading what is there.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question | Where to look |
|---|---|---|
| `strlen.counts_terminator` | Does the length `strlen` returns include the terminator? | DESCRIPTION, RETURN VALUE |
| `strlen.empty` | What does `strlen` return for the empty string? An integer. | DESCRIPTION (apply it) |
| `strlen.header` | Which header declares `strlen`? The file name only. | SYNOPSIS |
| `strlen.return_type` | What type does `strlen` return? As the SYNOPSIS writes it. | SYNOPSIS |
| `strlen.param_const` | Is its parameter `const`-qualified? | SYNOPSIS |
| `strlen.unterminated` | The bytes have no terminator. What does the standard say the call returns: `a number`, or `undefined`? | nowhere: see **Background** |
| `strlen.bounded_name` | Which function of the same family takes a maximum length as well as a pointer? The function name only. | `man -k "string length"`, then that function's page |
| `puts.appends_newline` | Does `puts` write a newline after the string? | DESCRIPTION |
| `puts.error_return` | What does `puts` return on an error? The macro name. | RETURN VALUE |

Then, in `ANSWER.md`, under the heading for each key: the page and the section you found it
in, and the supporting sentence (or SYNOPSIS line), **quoted exactly**. A quote is evidence; a
paraphrase is your opinion of what the page says. For `strlen.unterminated`, quote instead the
sentence that says what `strlen` requires of its argument, and add one line saying what the
page does not say.

`ANSWER.md` ends with two ungraded questions: what the pages mean by *size* as opposed to
*length*, and why `puts` returns an `int` at all.

## How to write an answer

One value per line after the `=`. Two examples in the same form, about a function from
another unit:

```
# Which header declares memcpy?  (the file name only)
memcpy.header = string.h

# What does memcpy return?  dst | src | nothing
memcpy.returns = dst
```

Answers are not case-sensitive, so `EOF` and `eof` are both accepted. Give a file name as
`string.h`, without angle brackets; a type as the SYNOPSIS spells it; a function name without
parentheses.

## Constraints

- Answer from the pages. Do not write a test program, do not run the grader to find out, and
  do not answer from the lesson: the exercise is finding the sentence.
- Every answer in `answers.txt` has a quote in `ANSWER.md`.
- If the page on your system does not say it (the macOS pages are terser, and older Linux
  pages use different section names), say so in `ANSWER.md` and quote the nearest thing it
  does say.
- `man` opens in a pager: `/RETURN` then Enter jumps to the next line containing RETURN, and
  `n` repeats the search. `q` quits.

## What the program checks

Two of the answers describe behaviour, so the program makes the call and compares what
happens with what you say the page promises. The other seven are facts about the
documentation; for those it compares a hash of your answer, so reading `main.c` does not give
them away. Each key is reported as `ok`, `WRONG` or `blank`, and the correct value is never
printed. A key you misspelled is listed as `unknown key`.

`ANSWER.md` is not graded by the program. It is the half of the exercise you grade yourself:
is each quote exact, and does it actually say what your answer says?

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with
`s00-e07-string-lookup: 0 of 9 answers correct`. When you are done it ends with
`s00-e07-string-lookup: all checks passed`.

One of the nine is not reachable by following links from `strlen(3)`: on Linux, neither its
SEE ALSO nor the `string(3)` index mentions it. `man -k` (also spelled `apropos`) searches
the one-line descriptions of every installed page, and `man -k "string length"` answers it in
one line. On macOS that function shares the `strlen(3)` page, so it is in front of you
already. Searching the manual, rather than following links through it, is the part of this
exercise worth keeping.

<details><summary>Hint</summary>

`man 3 string` is an index, not a description: it lists the family by job (Copy, Catenate,
Compare, Search, Separate). Useful for seeing the shape of `<string.h>` in one screen, and
for finding the page of a function whose exact name you have forgotten.

</details>
