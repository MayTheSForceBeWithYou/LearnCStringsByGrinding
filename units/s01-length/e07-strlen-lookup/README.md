# s01-e07-strlen-lookup `[LOOKUP]`

## Goal

Answer nine questions about `strlen` and `strnlen` **from the manual pages**, and quote the
sentence that supports each answer. Afterwards you know which of the two is portable ISO C and
which is POSIX, what a feature-test macro is and where to look one up, and you have read the
difference between a *string* and a *character sequence* in the library's own words.

## Background

These two functions answer the same question and belong to different worlds. `strlen` has been
in ISO C since C89 and is available everywhere. `strnlen` is **not in ISO C at all**: it was
added by POSIX.1-2008, and on glibc it is declared only when the program asks for that
standard. This course's build asks for it — `lib/dsalloc.h` defines `_POSIX_C_SOURCE
200809L` before any system header — which is why you can call `strnlen` in every exercise
here without doing anything.

That mechanism is worth understanding once, because it explains a whole class of "undefined
reference to `strnlen`" and "implicit declaration of function" errors. A libc header contains
declarations for several standards at once, wrapped in `#if` tests on **feature-test macros**:

```c
#if defined __USE_XOPEN2K8          /* set when _POSIX_C_SOURCE >= 200809L */
extern size_t strnlen (const char *__string, size_t __maxlen) ...
#endif
```

Define the macro before including the header and the declaration appears; forget it and the
function exists in the library but not in your translation unit. The Linux manual pages
document this per function in a block called **Feature Test Macro Requirements**, and
`man 7 feature_test_macros` is the whole story. macOS pages have no such block: Apple's libc
declares `strnlen` unconditionally, so the question has no answer *there* — which is itself
worth noticing, and the README below says what to do about it.

A section-3 page's sections and what each is for:

| You want to know | Look in |
|---|---|
| the header, the exact prototype, `const` and `restrict` | **SYNOPSIS** |
| which feature-test macro the declaration needs | **SYNOPSIS**, in the Feature Test Macro Requirements block (Linux) |
| what the function does to its arguments | **DESCRIPTION** |
| what comes back | **RETURN VALUE** (macOS: **RETURN VALUES**) |
| which standard defines it | **STANDARDS** (older Linux pages: **CONFORMING TO**) |

The pages you need:

| | Linux (man-pages) | macOS |
|---|---|---|
| `strlen` | `man 3 strlen` | `man 3 strlen` (shared with `strnlen`) |
| `strnlen` | `man 3 strnlen` (its own page; `strlen(3)` does not link to it) | the `strlen(3)` page |
| the vocabulary | `man 7 string_copying`, **Terms** | not available |
| feature-test macros | `man 7 feature_test_macros` | not available |

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question | Where to look |
|---|---|---|
| `strlen.empty` | What does `strlen` return for the empty string? An integer. | DESCRIPTION, applied |
| `strnlen.zero` | What does `strnlen("abc", 0)` return? An integer. | DESCRIPTION, applied |
| `strnlen.full_field` | What does `strnlen` return for a 4-byte field holding four characters and no terminator? An integer. | DESCRIPTION, RETURN VALUE |
| `strlen.in_iso_c` | Is `strlen` part of ISO C? `yes` or `no`. | STANDARDS |
| `strnlen.in_iso_c` | Is `strnlen` part of ISO C? `yes` or `no`. | STANDARDS |
| `strnlen.macro` | Which feature-test macro must be defined for glibc to declare `strnlen`? The macro's name only. | SYNOPSIS, the Feature Test Macro Requirements block |
| `strnlen.macro_value` | Which value must it have, at least? As the page writes it. | the same block |
| `strnlen.header` | Which header declares `strnlen`? The file name only. | SYNOPSIS |
| `strlen.return_type` | What type does `strlen` return? As the SYNOPSIS writes it. | SYNOPSIS |

**If you are on macOS**, the two macro questions cannot be answered from your pages, because
Apple's libc does not gate `strnlen`. Answer them from the Linux documentation if you have it,
or from this course's `docs/AUTHORING-INFRA.md`, which states the macro and the value the
build defines — and say in `ANSWER.md` that your platform's page has no such block. Noticing
that two systems document the same function differently is part of the exercise, not a failure
of it.

Then, in `ANSWER.md`, under the heading for each key: the page and the section you found it
in, and the supporting sentence (or SYNOPSIS line), **quoted exactly**. A quote is evidence; a
paraphrase is your opinion of what the page says. The file ends with two ungraded questions:
one about the words the two pages use for their arguments, and one about why `strnlen` reports
"no terminator" as `maxlen` rather than as an error value.

## How to write an answer

One value per line after the `=`. Two examples in the same form, about a function from another
unit:

```
# Which header declares memcpy?  (the file name only)
memcpy.header = string.h

# Is memcpy part of ISO C?  yes | no
memcpy.in_iso_c = yes
```

Answers are not case-sensitive. Give a file name as `string.h`, without angle brackets; a type
as the SYNOPSIS spells it; a macro name with its leading underscore.

## Constraints

- Answer from the pages. Do not write a test program, do not run the grader to find out, and
  do not answer from the lesson: the exercise is finding the sentence.
- Every answer in `answers.txt` has a quote in `ANSWER.md`.
- `strnlen(3)` is not reachable by following links from `strlen(3)` on Linux: neither its SEE
  ALSO nor the `string(3)` index mentions it. `man -k "string length"` (also spelled
  `apropos`) finds it in one line. Searching the manual rather than following links through it
  is a skill worth having.
- If a page on your system does not answer a key, say so in `ANSWER.md` and quote the nearest
  thing it does say.

## What the program checks

Three of the answers describe behaviour, so the program makes the call and compares what
happens with what you say the page promises — including `strnlen` on a four-byte field with no
terminator, which is the behaviour that makes the function worth having. The other six are
facts about the documentation; for those it compares a hash of your answer, so reading
`main.c` does not give them away. Each key is reported as `ok`, `WRONG` or `blank`, and the
correct value is never printed.

`ANSWER.md` is not graded by the program. It is the half of the exercise you grade yourself:
is each quote exact, and does it actually say what your answer says?

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with
`s01-e07-strlen-lookup: 0 of 9 answers correct`. When you are done it ends with
`s01-e07-strlen-lookup: all checks passed`.

<details><summary>Hint</summary>

The Feature Test Macro Requirements block sits between the SYNOPSIS and the DESCRIPTION, and
it is easy to scroll past because it looks like part of the prototype. It names the macro, the
value, and the glibc version from which that value has been enough.

</details>
