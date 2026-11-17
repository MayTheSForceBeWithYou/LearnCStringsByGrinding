# s04-r01-append-reps `[DRILL]` (XS)

## Skill

Appending at an end pointer: a string, a character, a decimal number, and the arithmetic that
says how much room is left. Four functions, one state (`p`, where the next character goes),
and no rescanning anywhere.

This is the motion under every string builder in the course — S04's cursor, S11's table
writer, S14's error formatter — and it is the one that makes the difference between a linear
append loop and e03's quadratic one. In real code it is what you write when a log line, a
path or a query is assembled from pieces into a fixed buffer.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `app.c`, below the marked line.

## Functions

```c
size_t app_room(const char *p, const char *end);
char *app_str(char *p, char *end, const char *s);
char *app_char(char *p, char *end, char c);
char *app_dec(char *p, char *end, long v);
```

`p` is where the next character goes; `end` points one past the buffer's last byte. Every
appender writes what fits, terminates, and returns the new `p`, so appends chain:

```c
p = app_str(p, end, "id=");
p = app_dec(p, end, 42);
p = app_char(p, end, ';');
```

| Function | Returns | Example in an 8-byte buffer |
|---|---|---|
| `app_room(p, end)` | `(end - p) - 1`, and 0 at `end` | `app_room(buf, buf + 8)` is 7 |
| `app_str` | the new `p` | `"abcdefghij"` into an empty buffer gives `abcdefg`, `p == end - 1` |
| `app_char` | the new `p`, unchanged if it did not fit | |
| `app_dec` | the new `p` | `-1234` into 4 bytes gives `-12` |

A function that cannot fit everything writes what it can, terminates, and returns `end - 1`,
from which nothing more will ever fit. `app_dec` produces its digits by hand — no `snprintf` —
and must be correct for `0`, for negative values, and for `LONG_MIN`, which cannot be negated.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s04-concatenation/drills/r01-append-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- No library call that appends: `strcat`, `strncat`, `strlcat`, `strlcpy`, `snprintf` and
  `sprintf` are in `FORBID`. `strlen` and `memcpy` are allowed.
- No rescanning: nothing may call `strlen` on the destination.
- `app_room` reads no memory. `app_char` and `app_dec` may use each other and `app_room`.
- Every function terminates the buffer before returning.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness checks `app_room` at four positions including both ends; a three-call chain;
an exact fit followed by one character too many; a truncating `app_str` and a second append
after it; `app_dec` for 0, a single digit, ten digits, a negative value and `LONG_MIN`
(compared against `snprintf("%ld")`); a truncating `app_dec` with and without a sign; a
one-byte buffer where nothing fits; and bytes above 0x7F. Every buffer is exactly its stated
size with fences around it. It prints `s04-r01-append-reps: all checks passed`.

The failure to expect on a first attempt is `app_dec` on `LONG_MIN`: negating it is undefined,
and the harness compares against `snprintf`, so the digits have to be produced without a
negation.
