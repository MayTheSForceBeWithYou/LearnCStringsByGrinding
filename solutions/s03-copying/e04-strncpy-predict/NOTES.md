# s03-e04-strncpy-predict: notes

## Why each answer is what it is

| Call | Buffer | Ret | Reason |
|---|---|---|---|
| `strncpy(buf, "hello", 8)` | `hello\0\0\0` | 0 | 8 bytes written: 5 characters, 3 NULs of padding |
| `strncpy(buf, "hello", 5)` | `hello###` | 0 | 5 bytes written, all characters; no room and no attempt to terminate |
| `strncpy(buf, "hello", 3)` | `hel#####` | 0 | 3 bytes written |
| `strncpy(buf, "", 4)` | `\0\0\0\0####` | 0 | 4 bytes written, all padding |
| `strncpy(buf, "abcdefgh", 8)` | `abcdefgh` | 0 | a full field: 8 characters, no NUL anywhere |
| `stpncpy(buf, "hey", 8)` | `hey\0\0\0\0\0` | 3 | same bytes as `strncpy`; returns the first NUL |
| `stpncpy(buf, "hello", 5)` | `hello###` | 5 | no NUL written; returns `buf + n` |
| `snprintf(buf, 8, "%s", "hello")` | `hello\0##` | 5 | 6 bytes written; no padding; returns the length |
| `snprintf(buf, 8, "%s", "abcdefghij")` | `abcdefg\0` | 10 | 7 characters and a terminator; returns the length it needed |
| `snprintf(buf, 4, "%s", "hello")` | `hel\0####` | 5 | the size is 4, so bytes 4 to 7 are not touched |

`strncpy` always returns `buf`, so its `.ret` is 0 in all five: a return value that carries
no information, which is the point of asking for it five times.

## Numbers worth noticing

Bytes written: `strncpy`/`stpncpy` wrote 8, 5, 3, 4, 8, 8, 5: always `n`. `snprintf` wrote 6,
8, 4: `min(len, size - 1) + 1`.

Safe to pass to `strlen` afterwards: calls 1, 4, 6, 8, 9, 10. Not safe: 2, 3, 5, 7. For those
`strlen` reads `#` bytes to the end of the buffer and then past it.

## Alternatives

None: the answers are facts. The alternative is in how you derive them. Count the bytes
written first (for two of the three functions that needs only `n`), then decide what each
byte is.

## The usual mistake

Call 2: writing `hello\0##`, from the belief that `strncpy` "copies the string" and so copies
its terminator when there is room in the *buffer*. Room in the buffer is irrelevant; the
function knows only `n`.

Call 9: answering 7 for the return value (what was stored) instead of 10 (what was needed).

## How libc does it

See the notes of e02 for `strncpy`. `snprintf` formats into the caller's buffer through a
counting output stream that discards everything past `size - 1` but keeps counting; that
count is the return value.
