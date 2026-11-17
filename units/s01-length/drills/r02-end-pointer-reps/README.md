# s01-r02-end-pointer-reps `[DRILL]` (XS)

## Skill

Keeping your place in a buffer with pointers instead of lengths: where the string ends, what
its last character is, how far apart two points are, and how much room is left before the
buffer does. This is the appender's arithmetic, and it is the shape every string-building
function in the course uses — `stpcpy` chains in S03, the cursor of S04 e05, the table writer
of S11 e06, and every `end - p` in between.

Two pointers are often better bookkeeping than two lengths, because the pair "where I am" and
"where the buffer ends" cannot drift out of step with the data the way a stored length can.
The cost is that the arithmetic has to be right, and that is what the reps are for.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `endreps.c`, below the marked line.

## Functions

```c
char *end_of(char *s);
char *last_char(char *s);
size_t len_between(const char *begin, const char *end);
size_t room_left(const char *p, const char *buf_end);
```

| Function | Returns | Example |
|---|---|---|
| `end_of` | a pointer to the terminator: `&s[strlen(s)]`, never NULL | `end_of("hi")` is `s + 2`, and `*end_of("")` is `'\0'` |
| `last_char` | a pointer to the last character, or NULL if `s` is empty | `last_char("hi")` is `s + 1`; `last_char("")` is NULL |
| `len_between` | the number of bytes from `begin` to `end` | `len_between(s, s + 3)` is 3; `len_between(s, s)` is 0 |
| `room_left` | how many more **characters** may be written at `p` before `buf_end` | in an 8-byte buffer, `room_left(buf, buf + 8)` is 7 |

`end_of` and `last_char` require `s` to be a string and read up to and including its
terminator. `len_between` requires both pointers to point into one object with `begin` no
later than `end`; it reads nothing at all.

`room_left(p, buf_end)` is the one to think about. `buf_end` points **one past** the buffer's
last byte, so `buf_end - p` is the number of bytes available at `p` — and one of them has to
be kept for the terminator. So the answer is `(buf_end - p) - 1` when there is at least one
byte, and 0 when `p` has reached `buf_end`. It reads no memory: it is arithmetic on two
pointers, which is exactly why an appender can ask it before every write.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s01-length/drills/r02-end-pointer-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- No library call: `strlen`, `strnlen`, `memchr`, `strchr`, `strrchr`, `strcspn` and `snprintf`
  are in `FORBID`.
- Pointers, not indexes: that is the drill. `last_char` may call `end_of`, and `room_left` may
  call `len_between`; those are the intended shapes.
- `room_left` reads no memory. If your version dereferences anything, it is answering a
  different question.

## Target

8 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs five strings through `end_of`, `last_char` and `len_between`, each in a
buffer of exactly its own size; checks `len_between` over four sub-ranges of one string;
walks `room_left` across an 8-byte buffer at five positions including both ends; then builds a
string with the pair `room_left`/`end_of`, copying a 10-character source into 8 bytes and
checking that the result is `abcdefg\0`; then 2000 generated strings. It prints
`s01-r02-end-pointer-reps: all checks passed`.

A failure in the `room_left` cases is almost always the reserved byte: `buf_end - p` counts
bytes, and a *character* count has to leave one for the terminator.
