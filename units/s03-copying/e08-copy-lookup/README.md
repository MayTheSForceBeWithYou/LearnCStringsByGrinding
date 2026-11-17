# s03-e08-copy-lookup `[LOOKUP]`

## Goal

Answer nine questions about the copy functions **from the manual pages**, and quote the
sentence that supports each answer. Afterwards you know where on a `man` page a function's
promises live, and you have read, in the library's own words, the warnings this unit has
been making.

## Background

Everything in this unit's lesson can be checked against a primary source that is already on
your machine. Being able to do that quickly is a skill in itself: when you meet a function
you have not used for a year, or one this course never covered, the page is what you have.

A section-3 manual page has a fixed shape, and each kind of question has a home:

| You want to know | Look in |
|---|---|
| which header to include, the exact prototype, `const` and `restrict` | **SYNOPSIS** |
| what the function does to its arguments | **DESCRIPTION** |
| what comes back, and how errors are signalled | **RETURN VALUE** (macOS: **RETURN VALUES**) |
| which standard defines it (ISO C? POSIX only?) | **STANDARDS** (older Linux pages: **CONFORMING TO**) |
| the traps | **CAVEATS**, **BUGS**, **NOTES** (macOS: **SECURITY CONSIDERATIONS**) |

The pages differ between systems, and you should know both layouts:

| | Linux (man-pages) | macOS |
|---|---|---|
| `strcpy` | `man 3 strcpy` (shared with `strcat`) | `man 3 strcpy` (one page for all four functions) |
| `stpcpy` | `man 3 stpcpy` | `man 3 strcpy` |
| `strncpy` | `man 3 strncpy` | `man 3 strcpy` |
| `stpncpy` | `man 3 stpncpy` | `man 3 strcpy` |
| overview | `man 7 string_copying` | none |

The two systems word things differently, and the answers below are the same on both.

## The questions

Answer in `answers.txt`; the allowed values are listed there next to each key.

| Key | Question | Where to look |
|---|---|---|
| `strcpy.returns` | What does `strcpy` return: `dst`, or a pointer to the `end` (the terminator it wrote)? | RETURN VALUE |
| `stpcpy.returns` | The same question for `stpcpy`. | RETURN VALUE |
| `strncpy.pads` | After `strncpy(buf, "ab", 6)`, are bytes 2 to 5 of `buf` NUL? | DESCRIPTION |
| `strncpy.terminates_full` | After `strncpy(buf, "abcdef", 6)`, is there a NUL anywhere in the six bytes written? | DESCRIPTION, CAVEATS |
| `stpncpy.full_offset` | `stpncpy(buf, "abcdef", 6)` returns `buf` plus how many? | RETURN VALUE |
| `strcpy.header` | Which header declares `strcpy`? Give the file name only. | SYNOPSIS |
| `strcpy.overlap_allowed` | May the source and destination passed to `strcpy` overlap? | CAVEATS; the `restrict` in SYNOPSIS says the same |
| `stpcpy.iso_c17` | Is `stpcpy` part of ISO C17, or only of POSIX? | STANDARDS |
| `strncpy.reports_truncation` | Can a caller tell from `strncpy`'s return value that the copy was truncated? | RETURN VALUE, CAVEATS |

Then, in `ANSWER.md`, under the heading for each key: the page and section you found it in,
and the supporting sentence (or SYNOPSIS line), **quoted exactly**. A quote is evidence; a
paraphrase is your opinion of what the page says. The file ends with one ungraded question
about what `strncpy` is *for*.

## How to write an answer

Two examples, about a function from another unit, in the same form:

```
# What does strlen return for the empty string?  (an integer)
strlen.empty = 0

# Which header declares strlen?  (the file name only)
strlen.header = string.h
```

Answers are not case-sensitive. Give `yes` or `no`, `dst` or `end`, an integer, or a file name
such as `stdio.h` without angle brackets, exactly as the comment above each key says.

## Constraints

- Answer from the pages. Do not write a test program, do not run the grader to find out, and
  do not answer from the lesson: the exercise is finding the sentence.
- Every answer in `answers.txt` has a quote in `ANSWER.md`.
- If the page on your system does not say it (older pages are terser), say so in `ANSWER.md`
  and quote the nearest thing it does say.

## What the program checks

Five of the answers describe behaviour, so the program makes the call and compares what
happens with what you say the page promises. The other four are facts about the
documentation; for those it compares a hash of your answer, so reading `main.c` does not
give them away. Each key is reported as `ok`, `WRONG` or `blank`; the correct value is never
printed. `ANSWER.md` is not graded by the program. It is the half of the exercise you grade
yourself: is each quote exact, and does it actually say what your answer says?

## Verify

```bash
make check
```

With the file as shipped, every line is `blank` and the run ends with
`s03-e08-copy-lookup: 0 of 9 answers correct`. When you are done it ends with
`s03-e08-copy-lookup: all checks passed`.

<details><summary>Hint</summary>

`man` opens in a pager. Type `/RETURN` and press Enter to jump to the next line containing
RETURN; `n` repeats the search.

</details>
