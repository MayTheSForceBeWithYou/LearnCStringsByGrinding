# s03-r03-stpcpy-chain-reps: notes

## Why it is correct

Each function keeps one pointer `p` that always points at the terminator of what has been
written so far. Appending is "copy the part to `p`, advance `p` by the part's length", which
`stpcpy` does in one call.

- `chain_two`: `stpcpy(stpcpy(dst, a), b)`.
- `chain_path`: `stpcpy`, then `*p++ = '/'` over the terminator, twice; the length is
  `p - dst` at the end.
- `chain_repeat`: writes the empty string first so that `n == 0` is correct, then `n` appends.
- `chain_bounded`: keeps `room`, the number of characters that still fit with one byte
  reserved for the terminator. A part fits when `len <= room`. Failure resets `dst[0]`.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| empty parts | `stpcpy` of `""` writes a terminator at `p` and returns `p` |
| `chain_repeat` with `n == 0` | `*p = '\0'` before the loop |
| `chain_bounded` with `cap == 0` | returns NULL before touching `dst` |
| twelve characters into twelve bytes | `room` starts at 11; the last part has `len > room` |
| no parts | the initial `*p = '\0'`; returns `dst` |

## Numbers

A million one-character appends: with an end pointer, 2 million byte writes. With
`strcat(dst, s)` each append first scans the current length, so the total is
1 + 2 + ... + 1,000,000, about 5 x 10^11 byte reads. Measured on the machine the course was
written on, plain build: the end-pointer version runs in about 6 ms; a `strcat` version
takes about 5 s of CPU and fails the 1-second check.

## Alternatives

`chain_bounded` with `snprintf(p, room + 1, "%s", parts[i])` and a check of its return value
works too, and is the form to use when the parts are not all strings (S04's cursor). A
`memcpy`-only version (`memcpy(p, part, len); p += len;` and one terminator at the end)
avoids writing a terminator per part.

## The usual mistake

Checking the room after writing. Comparing `len < room` (rejects an exact fit) or
`len <= cap - used` (forgets the terminator). Calling `strlen(dst)` "just to be sure", which
quietly puts the quadratic back.

## How libc does it

`stpcpy` is `strlen` plus `memcpy` plus the address arithmetic; there is nothing more to it.
The interesting part is that the standard C library has no bounded equivalent, which is why
every project grows its own (`stpecpy` in the Linux man pages' `string_copying(7)`, `seprintf`
in Plan 9).
