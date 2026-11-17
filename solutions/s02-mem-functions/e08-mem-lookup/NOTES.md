# s02-e08-mem-lookup: notes

## Why each answer is what it is

| Key | Answer | Section that settles it |
|---|---|---|
| `memcpy.returns` | `dst` | RETURN VALUE: "returns a pointer to dest" |
| `memmove.returns` | `dst` | RETURN VALUE, the same wording |
| `memchr.not_found` | `null` | RETURN VALUE: "or NULL if the character does not occur" |
| `memcmp.zero_n` | `0` | DESCRIPTION, applied: no bytes, so no differing byte |
| `memcpy.restrict` | `yes` | SYNOPSIS: `void dest[restrict n], const void src[restrict n]` |
| `memmove.restrict` | `no` | SYNOPSIS: `void dest[n], const void src[n]` |
| `memcpy.overlap_ok` | `no` | DESCRIPTION: "The memory areas must not overlap" |
| `memcmp.compares_as` | `unsigned char` | DESCRIPTION: "each interpreted as unsigned char" |
| `memset.converts_to` | `unsigned char` | DESCRIPTION, and C17 7.24.6.1 |

## Numbers worth noticing

Two SYNOPSIS lines, differing by one qualifier twice, are the whole difference between the two
functions — and that difference is a statement about the *caller*, not about the code. There
is nothing in `memcpy`'s description that `memmove`'s does not also do; there is a requirement
in `memcpy`'s that `memmove`'s does not have. C's type system has exactly one way to express
such a requirement, and this is it.

Three of the nine answers are the words `unsigned char`, from three different pages. That is
not a coincidence: the `mem` family is specified entirely in terms of `unsigned char`, which
is why none of its results depends on whether plain `char` is signed. Every place in this
course where signedness *does* bite — `isalpha(c)`, `table[c]`, a hand-written comparison —
is a place where the program, not the library, decided what type a byte has.

## Alternatives

Three sources, and what each is the authority on:

| Source | Authority on |
|---|---|
| the manual page | what this implementation does, its caveats, and its history |
| the C standard (C17 7.24) | what you may rely on across implementations |
| `/usr/include/string.h` | the declarations your translation unit actually sees |

For the `mem` functions the standard and the pages agree closely, and the pages add two things
the standard does not have: the CAVEATS story about glibc 2.13, and the ATTRIBUTES table
stating that these functions are MT-Safe. The standard, in exchange, is the place that spells
out the `n == 0` rule precisely (C17 7.24.1p2: the pointers must still be valid) — a detail the
Linux pages leave to the reader.

## The usual mistake

Answering the `restrict` questions from what the functions *do* rather than from the
prototypes. Both functions copy; only one of them promises. The SYNOPSIS is the only place
that distinction appears in the documentation, and it is two characters' difference in a line
most readers skim.

Second: answering `memcmp.zero_n` with "undefined" or "unspecified", on the grounds that the
page says nothing about it. The page says nothing because nothing needs saying: the
description is in terms of the first byte that differs, and when no bytes are compared there
is no such byte. (What *is* a requirement even at `n == 0` is that the pointers be valid —
`memcmp(NULL, NULL, 0)` is undefined, which surprises people and is in the standard rather
than the page.)

Third: quoting a paraphrase. "The page says memcpy's areas can't overlap" is a belief; "The
memory areas must not overlap." is the sentence.

## How libc does it

The pages document a family that is, in every modern libc, chosen at run time. glibc resolves
`memcpy`, `memmove`, `memset`, `memcmp` and `memchr` through *ifuncs* — a resolver that runs
at load time, reads the processor's feature bits and picks an implementation — so the
`memcpy` your program calls on one machine may be a different routine from the one it calls on
another, with different copy orders and different thresholds.

That is exactly why the contract matters more than the observed behaviour, and why the CAVEATS
story is in the page at all. The library reserves the right to copy your bytes in whatever
order the hardware prefers; the only thing that stays fixed is what it promised.

Two practical consequences, both worth carrying out of this unit. `memcmp` may read all `n`
bytes even after finding a difference, so it is not constant-time and must not be used on
secrets. And `memset` may be optimised away entirely when its result is never read, which is
why clearing sensitive memory needs `memset_explicit` (C23) or a platform equivalent rather
than a plain call.
