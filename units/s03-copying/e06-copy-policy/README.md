# s03-e06-copy-policy `[DESIGN]` (M)

## Goal

Design and implement three bounded copy functions that differ only in **policy**: what
happens when the source does not fit. You write each function's contract in the header
first, then the code that honours it. Afterwards you can look at any copy in a code base and
ask the question that matters, "what does this do when it does not fit, and does the caller
know?", and you have three answers ready to use.

## Background

The library's copy functions each answer the "does not fit" question differently, and none
of them lets the caller choose:

| Function | When the source does not fit |
|---|---|
| `strcpy` | undefined behaviour: writes past the buffer |
| `strncpy` | silently produces something that is not a string |
| `snprintf(dst, cap, "%s", src)` | truncates, terminates, and reports the needed length |
| `strlcpy` (not portable) | the same, as a `size_t` |

"Truncate" is the right policy for a log message and the wrong one for a file name: a
truncated path is a *different* path, and code that opens it, deletes it or grants access to
it is now operating on something the user never named. So real programs need at least two
policies, and they need them as named functions, so that the choice is visible at the call
site rather than buried in how a return value is or is not checked.

The third function covers a different situation: the text is not a string at all, but `n`
bytes inside something larger (a token, a field, a slice of a path). There is no terminator
to find, and looking for one would read past the slice.

A contract is not a comment that restates the code. It is the set of promises a caller may
rely on and the set of obligations a caller takes on. The test of a contract: could someone
write a correct call, and a correct test, from the header alone?

Read **Foundations: what "does not fit" can mean** and **Worked example 3** in `LESSON.md`
first.

## The API

`copypol.h` declares, and fixes:

```c
int copy_trunc(char *dst, size_t cap, const char *src);
int copy_strict(char *dst, size_t cap, const char *src);
int copy_n(char *dst, size_t cap, const char *src, size_t n);
```

In all three, `cap` is the total size of `dst` in bytes, terminator included: a caller passes
`sizeof buf`. The return codes are `COPY_OK` (0), `COPY_TRUNCATED` (1) and `COPY_NO_ROOM`
(-1).

**`copy_trunc(dst, cap, src)`** stores as much of the string `src` as fits.

- `COPY_OK` if `src` fits, that is, `strlen(src) < cap`.
- `COPY_TRUNCATED` if it does not fit and `cap > 0`: `dst` holds the first `cap - 1`
  characters of `src` and a terminator.
- `COPY_NO_ROOM` if `cap == 0`: `dst` is not touched.

**`copy_strict(dst, cap, src)`** stores the string `src` whole, or stores nothing.

- `COPY_OK` if `src` fits.
- `COPY_NO_ROOM` if it does not. If `cap > 0`, `dst` is then the **empty string**; it is
  neither a truncated copy nor its previous contents. If `cap == 0`, `dst` is not touched.

**`copy_n(dst, cap, src, n)`** stores the `n` characters at `src`, followed by a terminator.

- `src` need **not** be terminated. Exactly `n` bytes of it are read; `src[n]` is never read.
- `COPY_OK` if `n < cap`.
- `COPY_NO_ROOM` otherwise: empty string if `cap > 0`, untouched if `cap == 0`.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write the contracts in `copypol.h` (replace every `CONTRACT:` line), then your definitions in
`copypol.c`, below the marked line.

## Questions your contract must answer

The header marks nine places with `CONTRACT:`. Each is a question the prototype cannot answer
and a caller needs answered. Write one or two sentences for each, in the header, before any
code.

For every function:

1. **Which bytes of `dst` are written, and which are left alone?** In particular: is the
   buffer padded? On `COPY_NO_ROOM`, what is in `dst[1]` onwards?
2. **What is required of the arguments?** May `dst` or `src` be NULL? May the buffers
   overlap? Must `src` be terminated? If a requirement is "the caller guarantees it and the
   function does not check", say exactly that.

And one question particular to each:

3. `copy_trunc`: **when is truncation acceptable**, and what must a caller do with
   `COPY_TRUNCATED`? Give one kind of data for which ignoring it is fine and one for which it
   is a defect.
4. `copy_strict`: **why the empty string on failure**, rather than leaving `dst` as it was?
   What does a caller who forgets to check the return value end up with in each design?
5. `copy_n`: **what if one of the `n` bytes is a NUL?** And name one situation in which a
   caller has a length but no terminator.

**What is fixed and what is yours.** The return codes, the meaning of `cap`, and the contents
of `dst` up to and including its terminator are fixed, and the harness tests them. The answers
to questions 1 (beyond the terminator), 2, 3, 4 and 5 are your design decisions, with one
exception: the harness does test that a NUL among `copy_n`'s `n` bytes is copied like any
other byte. The harness cannot grade prose. Grade it yourself against the test above: could
someone write a correct call from your header alone?

## Behaviour

`dst` is an 8-byte buffer holding `########` before each call.

| Call | Returns | `dst` afterwards |
|---|---|---|
| `copy_trunc(dst, 8, "hello")` | `COPY_OK` | `hello\0` then your contract decides |
| `copy_trunc(dst, 8, "abcdefg")` | `COPY_OK` | `abcdefg\0` (exact fit) |
| `copy_trunc(dst, 8, "abcdefgh")` | `COPY_TRUNCATED` | `abcdefg\0` |
| `copy_trunc(dst, 1, "x")` | `COPY_TRUNCATED` | `\0#######` |
| `copy_trunc(dst, 0, "x")` | `COPY_NO_ROOM` | `########` (not touched) |
| `copy_strict(dst, 8, "abcdefg")` | `COPY_OK` | `abcdefg\0` |
| `copy_strict(dst, 8, "abcdefgh")` | `COPY_NO_ROOM` | `\0` then your contract decides |
| `copy_strict(dst, 1, "")` | `COPY_OK` | `\0#######` |
| `copy_strict(dst, 0, "")` | `COPY_NO_ROOM` | `########` (not touched) |
| `copy_n(dst, 8, "hello", 3)` | `COPY_OK` | `hel\0####` |
| `copy_n(dst, 8, "abcdefgh", 8)` | `COPY_NO_ROOM` | `\0` then your contract decides |
| `copy_n(dst, 8, p, 0)` | `COPY_OK` | `\0#######`; `p` is not read |
| `copy_n(dst, 8, "ab\0cd", 5)` | `COPY_OK` | `ab\0cd\0##` |
| `copy_n(dst, 8, p, SIZE_MAX)` | `COPY_NO_ROOM` | `\0` then your contract decides |

## Constraints

- Any `<string.h>` function may be used. Nothing is forbidden here: the exercise is the
  policy, and `strlen` and `memcpy` are the right tools.
- Never write `dst[i]` for `i >= cap`. With `cap == 0`, never touch `dst`.
- `copy_n` reads exactly `n` bytes of `src`; it must not call `strlen` on `src`.
- Decide whether the source fits **before** writing, in `copy_strict` and `copy_n`.
- `copy_n` must be correct for `n == SIZE_MAX`: no `n + 1` that wraps to 0.
- No allocation.

## What the harness checks

- **`copy_trunc`**: fits, exact fit, one too many, a long source, the empty string, `cap == 1`,
  `cap == 0`; return code and buffer contents each time.
- **`copy_strict`**: the same boundary cases; after each `COPY_NO_ROOM`, `dst[0] == '\0'`; and
  that the buffer is usable again by a following successful call.
- **`copy_n`**: sources that have **no terminator** and exactly `n` readable bytes (reading
  `src[n]` is an ASan report), a prefix, the exact fit, one too many, `n == 0` from a
  zero-byte source, an embedded NUL, `cap == 0`, and `n == (size_t)-1`.
- **Bounds**: every destination is exactly `cap` bytes, with a zero-byte buffer for
  `cap == 0`. In the plain build, fences around the buffer are checked; in the ASan build,
  the first byte out of bounds stops the program.

## Verify

```bash
make check
```

Success ends with `s03-e06-copy-policy: all checks passed` and `== check: OK`. A failed check
names the function and the case:

```
FAIL main.c:58: copy_strict: on COPY_NO_ROOM dst is the empty string, not the old value
```

Before you call it done, reread your nine contract sentences against your code. If the code
does something the header does not say, one of them is wrong.

<details><summary>Hint</summary>

For each function, write the condition for "fits" as a single comparison between a length and
`cap`, and check `cap == 0` before anything else.

</details>
