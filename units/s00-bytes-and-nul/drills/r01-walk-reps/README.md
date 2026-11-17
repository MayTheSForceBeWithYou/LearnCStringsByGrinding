# s00-r01-walk-reps `[DRILL]` (XS)

## Skill

Walking a string from its first byte to its terminator, with a moving pointer, and answering
a question on the way. This is the loop under `strlen`, `strchr`, `strcmp`, `strspn` and every
other function in `<string.h>`, and you will write it from memory in S01, S02, S06, S08 and
S09, and again in capstone C1 where the whole header is yours to produce. In real code it is
what you write when the library has no function for your question: "how many fields does this
line have", "is this identifier all digits", "where does the last path component start".

Five questions, one walk each, no library call. The point of the rep is that the loop and its
stopping condition become something your hands do while your head thinks about the question.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `walk.c`, below the marked line.

## Functions

```c
size_t walk_len(const char *s);
const char *walk_end(const char *s);
size_t walk_count(const char *s, char c);
int walk_last(const char *s);
int walk_is_empty(const char *s);
```

In all five, `s` is a string: the bytes from `s[0]` to its first NUL are readable, that NUL is
part of what you may read, and the byte after it is not.

| Function | Returns | Example |
|---|---|---|
| `walk_len(s)` | the number of bytes before the terminator | `walk_len("hello")` is 5; `walk_len("")` is 0 |
| `walk_end(s)` | a pointer to the terminator itself, never NULL | `walk_end("hi")` is `s + 2`, and `*walk_end("")` is `'\0'` |
| `walk_count(s, c)` | how many of the bytes before the terminator equal `c` | `walk_count("banana", 'a')` is 3; `walk_count("ab", '\0')` is 0 |
| `walk_last(s)` | the last character as a value from 0 to 255, or -1 if `s` is empty | `walk_last("hi")` is `'i'`; `walk_last("\xFF")` is 255 |
| `walk_is_empty(s)` | 1 if `s` is the empty string, else 0 | `walk_is_empty("")` is 1, and it reads exactly one byte |

Three details the harness holds you to. The terminator is **not** a character of the string,
so `walk_count(s, '\0')` is 0 and `walk_last` of the empty string is -1 rather than 0. A byte
above 0x7F is an ordinary character, so `walk_last` must return 255 and not -1 for a string
ending in `0xFF` — which means casting to `unsigned char` before returning. And nothing may
read past the first terminator: `"ab\0cd"` has length 2, contains no `c`, and ends in `b`.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s00-bytes-and-nul/drills/r01-walk-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep. If you get stuck,
  stop the clock, read, and start a new rep another day.
- **A moving pointer, not an index.** The harness cannot tell the difference; the drill is the
  pointer form, because that is the form the library is written in and the form that makes
  `walk_end` obvious. Use `s[i]` in r02 instead, where it is the natural shape.
- No library call: `strlen`, `strnlen`, `strchr`, `strrchr`, `memchr`, `strspn` and `strcspn`
  are in `FORBID`, and `walk.c` is compiled with `-fno-builtin` so the compiler cannot call
  one for you.
- One pass per function. Reading the string twice to answer one question is a different skill
  (and usually a defect in the making).

## Target

8 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs nine fixed strings (empty, one character, repeats, a tab, bytes above 0x7F)
through all five functions, with each string held in a buffer of exactly its own size so that
a walk which oversteps the terminator is reported rather than tolerated; then two strings with
bytes after the first terminator; then 2000 generated strings compared against a byte-by-byte
count. It prints `s00-r01-walk-reps: all checks passed`.

A `READ` report from AddressSanitizer in this drill means one of the five walked past the
terminator: almost always a loop that tests `p[1]` or advances before testing.
