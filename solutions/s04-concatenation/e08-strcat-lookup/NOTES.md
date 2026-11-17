# s04-e08-strcat-lookup: notes

## Why each answer is what it is

| Key | Answer | Section that settles it |
|---|---|---|
| `strcat.returns` | `dst` | RETURN VALUE: "These functions return dst." |
| `strncat.result` | `abcd` | DESCRIPTION: "at most ssize non-null bytes ... followed by a null character" |
| `strncat.bytes_written` | `3` | the same sentence: 2 characters + 1 terminator |
| `strncat.short_src` | `abcd` | "at most": the source's own terminator stops it first |
| `strcat.need` | `9` | `strlen(dst) + strlen(src) + 1` with 3 and 5 |
| `strncat.need` | `6` | `strlen(dst) + strnlen(src, ssize) + 1` with 3, 5 and 2 |
| `strncat.max_bytes` | `5` | `ssize` characters plus the terminator |
| `strncat.reports_truncation` | `no` | RETURN VALUE: "strncat() returns dst." |
| `strlcat.in_posix_2008` | `no` | STANDARDS: "None." |

## Numbers worth noticing

The two size formulas differ in exactly one term, and that term is the whole difference
between the functions: `strlen(src)` against `strnlen(src, ssize)`. Both end in `+ 1`, and in
both cases it is the terminator — which `strcat` copies from the source and `strncat` writes
itself.

`strncat.need` being 6 for a 5-character source is the clearest possible statement that the
third argument bounds the source: with `ssize` 2, the function needs room for two characters
however long the source is.

`strncat.max_bytes` is `ssize + 1`, and it is the number that makes `strncat(dst, src, cap)`
an overflow: with `cap` as the third argument, the function may write `cap + 1` bytes *past
the existing contents* of a `cap`-byte buffer.

`strlcat.in_posix_2008` is `no` and the function is nevertheless on both course platforms.
That gap — available, not standard — is why SPEC section 10 lists it as not callable here, and
why e02 asks you to write it. "My compiler accepts it" is not the same question as "may I rely
on it".

## Alternatives

Three sources and what each is the authority on:

| Source | Authority on |
|---|---|
| `man 3 strcat` / `strncat` | the size formulas, the equivalences, the return values |
| `man 7 string_copying` (Linux) | which family each function belongs to, and the vocabulary |
| the C standard (C17 7.24.3) | what every implementation must do |

For appending, the pages are notably better than the standard: C17 states the behaviour
correctly and says nothing about how large the destination must be, because that is the
caller's problem. The man-pages state the caller's problem as a formula. When two sources
differ in *usefulness* rather than in content, use both.

`man 7 string_copying`'s Terms section is worth one more read at this point in the course,
because it names what each function is for: `strcat` catenates a *string*, `strncat`
catenates a *null-padded character sequence* into a string, and the `strl*` family is listed
under "copy/catenate with truncation". Those three labels answer most "which one do I want"
questions faster than the individual pages do.

## The usual mistake

Answering `strncat.need` with 9, by using `strlen(src)` in the formula. The page says
`strnlen(src, ssize)`, and the difference is the entire point of the question: a bounded
append needs room for the bound, not for the source.

Second: answering `strncat.bytes_written` with 2. The terminator is a byte, it is always
written, and forgetting it is how `strncat(dst, src, cap - strlen(dst))` — one short of
correct — gets written.

Third: answering `strlcat.in_posix_2008` with `yes` because the function exists on the
machine. STANDARDS says "None"; HISTORY lists the BSDs and glibc 2.38. POSIX added it in 2024,
which is a later standard than this course's `_POSIX_C_SOURCE 200809L`.

Fourth: quoting a paraphrase in `ANSWER.md`. The formulas are the evidence here, and they are
printed in the pages verbatim.

## How libc does it

The equivalences the pages print are close to the implementations. glibc's `strcat` is
`strlen` then `strcpy` (that is, `memcpy` of `strlen(src) + 1` bytes); `strncat` is `strlen`,
`strnlen`, `memcpy`, and a terminator written by hand. Both reduce to the vectorised
primitives of S02, so the per-byte cost is small and the *shape* — a walk per call — is what
remains.

glibc's long refusal to add `strlcpy` and `strlcat`, recorded in the `strlcat(3)` page's
CAVEATS, is worth reading as a design argument rather than a historical note: the functions
read the whole source even when they truncate, so using them to avoid overflows turns a linear
program into a quadratic one when the input is attacker-controlled. The page's recommended
alternatives are `snprintf` with its return value checked, and the end-pointer sketches
(`stpecpy`, `strtcpy`) that this unit's e05 and drill r01 build by hand.
