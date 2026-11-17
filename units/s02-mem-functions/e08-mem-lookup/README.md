# s02-e08-mem-lookup `[LOOKUP]`

## Goal

Answer nine questions about the five `mem` functions **from the manual pages**, and quote the
sentence that supports each one. Two of them are about a qualifier in a prototype; one is
about a sentence that exists because a real libc change broke real programs. Afterwards you
know where a page states what a function requires of its *caller*, which is the half of a
contract that C cannot express in types.

## Background

The `mem` functions are where manual pages earn their keep, because the interesting part of
each contract is a requirement rather than a behaviour. `memcpy` does not do anything
different from `memmove`; it *requires* something `memmove` does not. The page says so twice:
in the prototype, with `restrict`, and in the DESCRIPTION, in a sentence of English.

Three things to look for as you read.

**`restrict` in a prototype is a promise by the caller.** C17 6.7.3.1 defines it in terms of
objects accessed through the pointer during the lifetime of the call; informally, "nothing
else touches these bytes while I am running". It is not checked, it generates no code of its
own, and its only effects are that the compiler may optimise more aggressively and that
breaking it is undefined behaviour. The Linux pages write it inside array-parameter notation
(`void dest[restrict n]`), which is the man-pages style for C2x-era prototypes; macOS writes
`void *restrict dest`. Both mean the same thing.

**`memcpy(3)`'s CAVEATS section is a case study.** It records what happened when glibc 2.13
changed the order in which `memcpy` copied bytes on x86-64: programs that had been passing
overlapping buffers for years, and getting away with it, stopped working — and glibc 2.14 had
to add a versioned symbol so that old binaries kept the old behaviour. Few manual pages tell a
story; this one is worth reading twice, because it is the clearest statement anywhere of what
"undefined behaviour that works" is worth.

**Two functions take their byte as an `int`.** The pages say the value is converted to
`unsigned char` — `memset` for what it writes, `memchr` for what it looks for — and `memcmp`
says the bytes it compares are interpreted as `unsigned char`. Those three sentences are why
the sign of plain `char` never affects a `mem` function's result.

The pages you need:

| | Linux (man-pages) | macOS |
|---|---|---|
| `memcpy` | `man 3 memcpy` | `man 3 memcpy` |
| `memmove` | `man 3 memmove` | `man 3 memmove` |
| `memset`, `memcmp`, `memchr` | one page each | one page each |
| the family index | `man 3 string`, section **Memory** of `man 3 memory`(3head) | `man 3 string` |

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question | Where to look |
|---|---|---|
| `memcpy.returns` | What does `memcpy` return: `dst` or something else? | RETURN VALUE |
| `memmove.returns` | The same for `memmove`. | RETURN VALUE |
| `memchr.not_found` | What does `memchr` return when the byte is not among the `n`? `null` or `other`. | RETURN VALUE |
| `memcmp.zero_n` | What does `memcmp` return when `n` is 0? An integer. | DESCRIPTION, applied |
| `memcpy.restrict` | Are `memcpy`'s two pointer parameters `restrict`-qualified in the SYNOPSIS? `yes` or `no`. | SYNOPSIS |
| `memmove.restrict` | Are `memmove`'s? `yes` or `no`. | SYNOPSIS |
| `memcpy.overlap_ok` | May the two memory areas passed to `memcpy` overlap? `yes` or `no`. | DESCRIPTION, CAVEATS |
| `memcmp.compares_as` | As which type does `memcmp` compare the bytes? The type name, two words. | DESCRIPTION |
| `memset.converts_to` | To which type does `memset` convert its value argument? The type name, two words. | DESCRIPTION |

`memcmp.zero_n` is the one question whose answer is not written down anywhere on the page:
work it out from the DESCRIPTION's wording about the first differing byte, and say in
`ANSWER.md` which sentence you reasoned from.

Then, in `ANSWER.md`, under the heading for each key: the page and the section you found it
in, and the sentence (or SYNOPSIS line), **quoted exactly**. The file ends with two ungraded
questions: a summary of the glibc 2.13 story, and why `memcpy` exists at all when `memmove`
can do everything it can.

## How to write an answer

One value per line after the `=`. Two examples in the same form, about a function from another
unit:

```
# What does strcpy return?  dst | other
strcpy.returns = dst

# Which header declares strlen?  (the file name only)
strlen.header = string.h
```

Answers are not case-sensitive. Write a two-word type name with a single space:
`unsigned char`.

## Constraints

- Answer from the pages. Do not write a test program, and do not answer from the lesson: the
  exercise is finding the sentence.
- Every answer in `answers.txt` has a quote in `ANSWER.md`.
- If your platform's page words something differently, quote your platform and say so. The
  nine answers are the same on both; only the wording differs.
- The two `restrict` questions are about the **SYNOPSIS**, not about what the function does.
  Read the prototypes character by character.

## What the program checks

Four of the answers describe behaviour, so the program makes the call and compares what
happens with what you say the page promises. The other five are facts about the
documentation; for those it compares a hash of your answer, so reading `main.c` does not give
them away. Each key is reported as `ok`, `WRONG` or `blank`, and the correct value is never
printed.

`ANSWER.md` is not graded by the program. It is the half of the exercise you grade yourself:
is each quote exact, and does it actually say what your answer says?

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with
`s02-e08-mem-lookup: 0 of 9 answers correct`. When you are done it ends with
`s02-e08-mem-lookup: all checks passed`.

<details><summary>Hint</summary>

`man 3 memcpy` answers five of the nine on its own, and its CAVEATS section is the longest
piece of prose in any page this course asks you to read.

</details>
