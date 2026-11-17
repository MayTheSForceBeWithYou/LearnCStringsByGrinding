# s03-r01-bounded-copy-reps `[DRILL]` (XS)

## Skill

Copy a string into a buffer of `cap` bytes so that the result is always a string and the
caller always learns whether it was cut. You will write this in nearly every later exercise
that fills a caller's buffer (S04's appenders, S11's formatters, S12's line readers, S14's
error messages), and in real code it is the line that stands between user input and a stack
buffer. There are four idiomatic ways to write it; a practised C programmer can produce any
of them without thinking, and knows what each one costs.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `bcopy.c`, below the marked line.

## Functions

In all four, `cap` is the total size of `dst`, terminator included, and the harness always
passes `cap >= 1`. None of them pads: bytes after the terminator are left alone.

```c
size_t bc_loop(char *dst, size_t cap, const char *src);
int bc_format(char *dst, size_t cap, const char *src);
int bc_measured(char *dst, size_t cap, const char *src);
int bc_strict(char *dst, size_t cap, const char *src);
```

| Function | How it must be written | Returns | Example: `"abcdefgh"` into 8 bytes |
|---|---|---|---|
| `bc_loop` | one loop over `src`, no library call at all | `strlen(src)`; a result `>= cap` means truncated | `dst` = `abcdefg\0`, returns 8 |
| `bc_format` | `snprintf` with `"%s"` | 1 if truncated, else 0 | `dst` = `abcdefg\0`, returns 1 |
| `bc_measured` | `strlen`, then one `memcpy`, then the terminator | 1 if truncated, else 0 | `dst` = `abcdefg\0`, returns 1 |
| `bc_strict` | `strlen`, then decide, then copy or refuse | 0 and the whole string, or -1 and `dst` = `""` | `dst` = `\0`, returns -1 |

The first three truncate to `cap - 1` characters. `bc_strict` never truncates: it stores all
of `src` or the empty string. With `"hello"` into 8 bytes all four store `hello\0`; the first
returns 5 and the others 0.

What each costs: `bc_loop` reads the source once; `bc_measured` and `bc_strict` read it twice
(once to measure, once to copy) but copy with `memcpy`, which is much faster per byte than a
character loop; `bc_format` pays for parsing a format string on every call. All are fine. Know
which one you are writing.

## Rules

- Start every rep from a fresh copy: `sh scripts/rep.sh units/s03-copying/drills/r01-bounded-copy-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep. If you get stuck,
  stop the clock, read, and start a new rep another time.
- Each function is written the way the table says. The harness checks results, not methods;
  the method is the drill, so hold yourself to it.
- No `strcpy`, no `strncpy`.

## Target

12 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs nine sources (fits, exact fit, one too many, far too long, empty, `cap == 1`,
and a multi-byte character cut in half) through all four functions, in destinations of
exactly `cap` bytes, and then checks that none of them pads. It prints
`s03-r01-bounded-copy-reps: all checks passed`.
