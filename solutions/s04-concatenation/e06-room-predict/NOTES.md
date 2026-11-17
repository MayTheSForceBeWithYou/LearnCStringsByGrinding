# s04-e06-room-predict: notes

## Why each answer is what it is

| Key | `.buf` | `.ret` | `.room` | Reason |
|---|---|---|---|---|
| `call01` | `abcd\0###` | 0 | 3 | `strcat` appends 2 characters and a terminator; it returns `dst` |
| `call02` | `abcdefg\0` | 0 | 0 | the buffer is now exactly full |
| `call03` | `abcde\0##` | 0 | 2 | `strncat` with `n` 3 appends 3 characters, then a terminator |
| `call04` | `abcd\0###` | 0 | 3 | `n` 5 is more than the source has: it stops at the source's end |
| `call05` | `abcdefg\0` | 0 | 0 | 5 characters appended: 2 + 5 = 7, which fills the buffer |
| `call06` | `abcdefg\0` | 0 | 0 | `n` 1 appends `g` at index 6 and terminates at index 7 |
| `call07` | `abcd\0###` | 4 | 3 | `snprintf` returns the length of `"abcd"` |
| `call08` | `abcdefg\0` | 10 | 0 | the full result is 10 characters; 7 were stored |
| `call09` | `abcdefg\0` | 8 | 0 | 8 wanted, 7 stored: `8 >= 8` is the truncation signal |
| `call10` | `abcd\0###` | 4 | 3 | the `strlcat` contract: the length it tried to create |
| `call11` | `abcdefg\0` | 10 | 0 | the same number `snprintf` gave, as a `size_t` |
| `call12` | `abcdefg\0` | 8 | 0 | `8 >= 8`: truncated, and 9 bytes would have been enough |

## Numbers worth noticing

`call05` and `call06` both fill the buffer exactly, from different directions: one appends
five characters to two, the other appends one to six. Both write a terminator at index 7, the
last byte. Neither reports anything — `strncat` returns `dst` in both cases, exactly as it
does for `call03`, where three characters fitted easily. **The return value of `strncat` is
the same whether the append was comfortable, exact or cut**, which is the whole reason the
bottom four rows of the table exist.

`call08` and `call11` return 10 for a buffer that holds 7 characters. That is the point of
both contracts: the number describes the result that was wanted, so `ret + 1` (11) is the
capacity that would have been enough, and `ret >= cap` is a truncation test that needs no
other information.

`call09` and `call12` are the boundary. The result wants 8 characters, the buffer holds 8
bytes, so 7 characters fit and the comparison `8 >= 8` is true. A caller who wrote `>` instead
of `>=` would call this a success and lose the last character silently.

The `.room` column is 0 for six of the twelve calls, and in every one of those cases the next
append — of any length — appends nothing. `room` reaching 0 is the end of the buffer's useful
life, whatever the caller does next.

## Alternatives

None: these are facts about four functions. The derivation that does not slip is three steps
per call:

1. Write the eight boxes with the "before" contents, terminator marked.
2. Decide **how many characters** the call appends: all of the source (`strcat`,
   `snprintf`, `strlcat`-style, when it fits), `min(n, strlen(src))` (`strncat`), or
   `cap - 1 - have` (the two that truncate).
3. Put the terminator after them, and read the room off the picture.

The return value is then separate, and depends only on which function was called, not on what
fitted.

## The usual mistake

Answering `.ret` for `strncat` as the number of characters appended. It returns `dst` — offset
0 — in every case, and that is precisely the complaint against it: a bounded function whose
return value carries no information about the bound.

Second: answering `call08.ret` as 7, the number stored. Both `snprintf` and the `strlcat`
contract return what was *wanted*. If they returned what was stored, truncation would be
undetectable: a full buffer and a cut one both hold 7 characters.

Third: forgetting the terminator in `.room`. Eight bytes holding `abcd` have three characters
of room, not four: index 7 is reserved for the terminator of whatever comes next.

Fourth: writing `.buf` for `call04` as `abcd\0\0\0\0`, by analogy with `strncpy`'s padding.
`strncat` does not pad — it appends what it took and writes exactly one terminator. The bytes
after it keep whatever they held, which here is the `#` fill.

## How libc does it

The four implementations are the same two or three library primitives in different orders:
`strcat` is `strlen` + `memcpy`; `strncat` is `strlen` + `strnlen` + `memcpy` + a terminator;
`snprintf` formats into a counting output stream that discards everything past `size - 1` but
keeps counting, which is where its return value comes from; and the BSD `strlcat` is a bounded
walk, a copy and a second walk over the source to finish the count.

The interesting difference is not speed but what each one is *able* to tell you. `strcat` and
`strncat` are specified to return `dst` — a decision from an era when the useful information
was "the call succeeded", and every call succeeded. `snprintf` and `strlcat` came later, and
both chose to return a length, because by then the question that mattered had become "did it
all fit?".
