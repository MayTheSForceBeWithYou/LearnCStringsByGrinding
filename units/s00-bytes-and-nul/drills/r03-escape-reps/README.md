# s00-r03-escape-reps `[DRILL]` (XS)

## Skill

Rendering a buffer so that you can see what is really in it: the terminator, the control
characters, and the bytes above 0x7F that a terminal would swallow. This is the debugging tool
you will want for the rest of the course, and the course's own version of it
(`lib/bytes.h`) prints every buffer in every failure message you will read. Writing it
yourself, three times, means you can produce one in any code base in five minutes, which is
usually the fastest route out of "but the string looks right when I print it".

It also drills the `snprintf` contract — *write what fits, terminate, return what you needed*
— two units before `snprintf` has its reference card. That contract is the single most
reusable interface shape in C string work, and S03, S04, S11 and S14 all assume you have it
in your hands.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `esc.c`, below the marked line.

## Functions

```c
int esc_hex_digit(unsigned v);
size_t esc_one(char *out, size_t cap, unsigned char b);
size_t esc_buf(char *out, size_t cap, const void *p, size_t n);
size_t esc_str(char *out, size_t cap, const char *s);
```

The rendering, byte by byte:

| Byte | Rendered as |
|---|---|
| `0x00` | `\0` |
| `0x0A`, `0x09`, `0x0D` | `\n`, `\t`, `\r` |
| `0x5C`, a backslash | `\\` |
| `0x20` to `0x7E` | the character itself |
| anything else | `\xHH`, two **upper-case** hex digits |

| Function | Does | Returns | Example |
|---|---|---|---|
| `esc_hex_digit(v)` | the hex digit for 0 to 15 | `'0'`-`'9'`, `'A'`-`'F'`, or -1 for any other value | `esc_hex_digit(10)` is `'A'` |
| `esc_one(out, cap, b)` | renders one byte | the characters the full rendering needs | `esc_one(out, 8, 0xE9)` writes `\xE9`, returns 4 |
| `esc_buf(out, cap, p, n)` | renders `n` bytes, nothing between them | the same | `esc_buf(out, 8, "hi", 3)` writes `hi\0`, returns 4 |
| `esc_str(out, cap, s)` | renders the string **and its terminator** | the same | `esc_str(out, 8, "hi")` writes `hi\0`, returns 4 |

All three rendering functions follow one contract, which is `snprintf`'s:

- they write at most `cap` bytes into `out`, never `out[cap]`;
- when `cap > 0` the result is always terminated, even if it had to be cut;
- when `cap == 0` nothing at all is written, and `out` may not even be read;
- the return value is the length of the **complete** rendering, not of what was stored, so
  `ret >= cap` means the text was cut short.

That last point is what makes the function usable: a caller can ask for the size it needs by
calling with `cap == 0`, and can tell afterwards whether it got everything.

`esc_str` differs from `esc_buf` only in that it includes the terminator in what it renders,
which is the point: a tool for looking at strings has to show you the byte whose absence is
the bug.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s00-bytes-and-nul/drills/r03-escape-reps`.
- No lookups during a rep: not the lesson, not `man`, not `lib/bytes.c` — which is a
  reference implementation of exactly this and would make the rep pointless.
- No library call: `snprintf`, `sprintf`, `strcpy`, `stpcpy`, `strncpy`, `strcat`, `memcpy`
  and `strlen` are in `FORBID`. Counting characters you could not store is the skill; a
  library call that does it for you is the thing being drilled.
- One pass over the input. Rendering it twice (once to measure, once to write) also works and
  is a legitimate design, but here the return value has to come out of the same pass.
- The hex digits are upper case, and `\xHH` always has exactly two of them.

## Target

15 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness checks `esc_hex_digit` at 0, 9, 10, 15, 16 and 255; thirteen single bytes
(printable, space, tilde, the four named escapes, a backslash, `0x1F`, `0x7F`, `0x80`, `0xE9`,
`0xFF`); then four buffers rendered at **every** capacity from 0 to two more than they need,
checking the return value and the stored text each time; then `esc_str` including a cut one;
then 1500 generated buffers compared byte for byte against `lib/bytes.h`, which follows the
same contract. It prints `s00-r03-escape-reps: all checks passed`.

The capacity sweep is where a first attempt usually fails. If the return value is right but
the stored text is wrong at small capacities, the counting and the storing are not separated;
if `cap == 0` fails, something is written before the capacity is checked.
