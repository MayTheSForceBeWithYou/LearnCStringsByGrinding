# s05-e04-compiler-warnings: notes

## Why each answer is what it is

| # | Warns | Flag | Why |
|---|---|---|---|
| 01 | no | — | `strncpy(dst, src, 8)` with both pointers as parameters: the compiler knows neither size |
| 02 | yes | `-Wstringop-truncation` | the bound equals the destination's size and the source is a literal of exactly that length, so the terminator is known to be dropped |
| 03 | no | — | `return sizeof p` is a legitimate expression; nothing says the caller will misuse it |
| 04 | yes | `-Wsizeof-pointer-memaccess` | the same `sizeof` **used as a byte count** in a `mem` function, which gcc recognises as a pattern |
| 05 | no | — | the cast to `const char *` makes the types match; `-Wformat` compares types, not intentions |
| 06 | yes | `-Wformat` | `%d` against a `size_t`: a type mismatch the compiler can see from the format string |
| 07 | yes | `-Warray-bounds` | both sizes are known: a 23-byte literal into a `char[8]` |
| 08 | no | — | the source is a parameter, so the output's length is unknown; `-Wformat-truncation` needs a known length |
| 09 | yes | `-Wunterminated-string-initialization` | a four-byte literal into a three-byte array drops the terminator |
| 10 | yes | `-Wstringop-overread` | the array is a visible `static const` with no zero byte, so `strlen` is known to read past it |

Six of ten warn, and the six have one thing in common: in every one of them the compiler can
see a **constant** — a literal's length, an array's declared size, a format string's
conversion. The four silent ones all hide one of those behind a parameter.

## The pairs worth comparing

**03 against 04.** Both apply `sizeof` to a pointer. In 03 the value is simply returned, which
is a perfectly ordinary thing to want; there is nothing to warn about, because the mistake is
in what the *caller* does with it. In 04 the same value is the third argument of `memset`, and
gcc has a pattern for exactly that shape: "the argument to `sizeof` is the same expression as
the destination". A diagnostic fires on a *use*, not on a value.

**02 against 01.** The same function with the same bound, and only the one with visible
constants warns. Move a size behind a parameter and the analysis has nothing to work with —
which is precisely the hiding place S02 e04 and S04 e04 use, and the reason those two defects
are `[BUG]` exercises rather than compiler errors.

**07 against 08.** Both write into a local `char[8]`. In 07 the source is a literal, so the
overflow is a certainty and gcc says so. In 08 the source is a parameter, so the output
*might* fit; `snprintf` truncates safely in any case, and a warning would be noise. Compare
the lesson's `lesson-demo/capture-warn.txt`, where an `snprintf` with a literal source *does*
draw `-Wformat-truncation=`.

**05** is the one with no good answer. Passing `(const char *)&n` for a `%s` is a cast that
tells the compiler "trust me", and it does. At run time `printf` reads the four bytes of an
`int` as a string and keeps going. No compiler diagnoses it, no sanitizer catches it until
the read leaves the object, and the fix is not to write the cast.

`-Wstringop-overflow` is the flag in the README's table that is never the right answer here —
07 is reported under `-Warray-bounds=` on this gcc instead.

## Alternatives

Flags that would have caught more of the ten, and why they are not on:

| Flag | Would catch | Why it is not in `-Wall -Wextra` |
|---|---|---|
| `-Wconversion` | narrowing conversions (S01 e06) | thousands of warnings in correct code |
| `-Wformat=2` / `-Wformat-security` | a non-literal format string (S11 e03) | many false positives in logging wrappers |
| `-Wanalyzer-*` (gcc) | some cross-function cases | slow, and still misses most |
| `-fsanitize=address` at run time | all ten, when the bad path runs | needs the input that triggers it |

The last row is the honest summary of this exercise: the compiler sees *constants*, the
sanitizer sees *executions*, and neither sees a defect that is neither constant nor executed.
That is why this course builds everything twice and writes tests for the cases the tools
cannot reach.

## The usual mistake

Assuming that a function with a bound (`strncpy`, `snprintf`) is never diagnosed and one
without (`strcpy`) always is. Snippet 02 is a bounded copy that warns and snippet 08 is a
bounded write that does not; what decides it is whether the compiler can see the numbers.

Second: answering `-Wstringop-overflow` for 07. The defect is one gcc can describe either
way, and on this version it reports an out-of-bounds `__builtin_memcpy` under
`-Warray-bounds=`. Which flag a given gcc picks for a given shape is not stable across
versions — which is itself worth knowing before you put `-Werror=stringop-overflow` in a
build system.

Third: predicting 05 as a warning because it is obviously wrong. It is obviously wrong to a
reader; to the compiler it is a `const char *` argument for a `%s` conversion, which is
exactly what the format asked for.

## How libc does it

Some of these diagnostics are not the compiler's alone. glibc's headers declare the string
functions with `__attribute__((access(...)))` and, under `_FORTIFY_SOURCE`, redirect them to
`__strcpy_chk` and friends — which is how clang reports snippet 07 under `-Wfortify-source`
rather than `-Warray-bounds`. The diagnostic comes from the header's annotation meeting the
compiler's knowledge of the buffer.

That cooperation is also what e06 of this unit is about: the same annotations, with
optimisation on, insert a **run-time** check, so a copy whose destination size is known at
compile time but whose source length is not can still be stopped — by libc, at run time,
with a message that names neither the buffer nor the line.
