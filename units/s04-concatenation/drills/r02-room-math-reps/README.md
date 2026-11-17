# s04-r02-room-math-reps `[DRILL]` (XS)

## Skill

The four questions that come before every append — how much room is left, will this fit, how
big must the buffer be for two parts, for three — written so that no expression can wrap.
There are no buffers in this drill: it is `size_t` arithmetic, and the cases that matter are
the extremes.

Every bounded copy and every append in the course rests on these four lines, and getting them
wrong is not a style problem: an unsigned subtraction that wraps turns a rejected request into
a move of eighteen quintillion bytes. S01 e03 is the same mistake in a loop bound; here it is
in a capacity test.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `room.c`, below the marked line.

## Functions

```c
size_t room_left(size_t cap, size_t len);
int will_fit(size_t cap, size_t len, size_t add);
size_t size_needed(size_t a, size_t b);
size_t size_needed3(size_t a, size_t b, size_t c);
```

`cap` is always a buffer's total size in bytes, terminator included; `len` and the part
lengths are counts of characters.

| Function | Means | Examples |
|---|---|---|
| `room_left(cap, len)` | how many more characters fit | `room_left(8, 3)` is 4; `room_left(8, 8)` is 0; `room_left(0, 0)` is 0 |
| `will_fit(cap, len, add)` | can `add` more characters be appended | `will_fit(8, 3, 4)` is 1; `will_fit(8, 3, 5)` is 0; `will_fit(0, 0, 0)` is 0 |
| `size_needed(a, b)` | bytes for a two-part string: `a + b + 1` | `size_needed(3, 4)` is 8; `size_needed(0, 0)` is 1 |
| `size_needed3(a, b, c)` | bytes for three parts and two one-character separators | `size_needed3(3, 4, 5)` is 15 |

The two `size_needed` functions report an impossible size as **0**, which is their only error
signal: a sum that cannot be represented in a `size_t` has no right answer, and 0 is not a
legal buffer size for a string (even the empty string needs 1).

Every function must give the right answer when `len` is somehow larger than `cap`, when `cap`
is 0, and when the lengths are near `SIZE_MAX` — which means no expression may be evaluated
before it is known not to wrap.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s04-concatenation/drills/r02-room-math-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- `strcat`, `strncat`, `strlcat`, `snprintf` and `sprintf` are in `FORBID`; nothing here
  should want them anyway.
- **No expression may wrap**, including the intermediate ones. `a + b` with both near
  `SIZE_MAX` is a wrap even if you then compare the result.
- The later functions may use the earlier ones.
- `<stdint.h>` gives you `SIZE_MAX`.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs thirty-four cases with no buffers at all: `room_left` and `will_fit` at
`cap` 0 and 1, at a full buffer, with `len > cap`, at `SIZE_MAX`, and with an `add` of
`SIZE_MAX`; `size_needed` and `size_needed3` at 0, at ordinary sizes, at the largest values
that still fit, and at several that do not; and finally the four used together as an appender
would use them. It prints `s04-r02-room-math-reps: all checks passed`.

A failure on one of the `SIZE_MAX` cases is always the same thing: an addition or a
subtraction performed before the test that would have made it safe. Move the term to the other
side of the comparison — `b > SIZE_MAX - a` instead of `a + b > SIZE_MAX` — and it cannot
happen.
