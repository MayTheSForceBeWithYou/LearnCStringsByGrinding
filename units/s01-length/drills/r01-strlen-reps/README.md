# s01-r01-strlen-reps `[DRILL]` (XS)

## Skill

Measuring a string, three ways, and then using the measurement for the only question that
matters: does it fit? The three shapes are an index and a counter, a pointer difference, and a
bounded walk; the fourth function is the comparison `len < cap` that every bounded copy in
this course begins with.

You will write these in S02, S03, S04, S06, S08 and S09, and again in capstone C1 where the
whole of `<string.h>` is yours to produce. In real code the bounded one is the function you
reach for the moment data comes from outside the program, and the `len < cap` test is the line
between a log message and a stack smash.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `lenreps.c`, below the marked line.

## Functions

```c
size_t len_index(const char *s);
size_t len_diff(const char *s);
size_t len_bounded(const char *s, size_t maxlen);
int len_fits(const char *s, size_t cap);
```

| Function | How it must be written | Example |
|---|---|---|
| `len_index` | a subscript and a counter: `s[i]`, `i++` | `len_index("hello")` is 5 |
| `len_diff` | one moving pointer; the answer is the difference from the start | `len_diff("hello")` is 5 |
| `len_bounded` | a loop with two reasons to stop, the bound tested first | `len_bounded("hello", 3)` is 3; `len_bounded("A123", 4)` on an unterminated field is 4 |
| `len_fits` | the comparison, written so that it is correct for `cap == 0` | `len_fits("hello", 6)` is 1; `len_fits("hello", 5)` is 0; `len_fits("", 0)` is 0 |

The first two require `s` to be a string and read up to and including its terminator. The
third does **not**: `s` may be a character sequence with no terminator at all, and
`len_bounded` must not read `s[maxlen]`. `maxlen` may be 0, and then no byte is read.

`len_fits(s, cap)` returns 1 when the string `s`, with its terminator, fits in `cap` bytes —
that is, when `strlen(s) < cap`. Nothing fits in 0 bytes, not even the empty string.
`len_fits` must not read past the first `cap` bytes of `s` either: a string far longer than
the buffer must be rejected without walking all of it, which means the bounded function is the
one to build it on.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s01-length/drills/r01-strlen-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep. If you get stuck,
  stop the clock, read, and start a new rep another day.
- No library call: `strlen`, `strnlen`, `memchr`, `strchr`, `strcspn`, `strspn` and `snprintf`
  are in `FORBID`, and `lenreps.c` is compiled with `-fno-builtin` so your loop stays a loop.
- Each function is written the way the table says. The harness checks results, not methods;
  the method is the drill.
- One pass each. `len_fits` may call `len_bounded`, and that is the intended shape.

## Target

8 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs five strings (empty, one character, a word, a long sentence, one with bytes
above 0x7F) through all four functions, each string in a buffer of exactly its own size; then
three full fields with no terminator, with `len_bounded` called at every `maxlen` from 0 to
the width; then the empty string against `cap == 1` and `cap == 0`; then a string with an
embedded NUL; then 2000 generated strings compared against libc at a generated bound. It
prints `s01-r01-strlen-reps: all checks passed`.

A `READ` report from AddressSanitizer in this drill means `len_bounded` (or `len_fits` through
it) tested the byte before testing the bound.
