# s03-r02-fixed-field-reps `[DRILL]` (XS)

## Skill

Work with a NUL-padded fixed-width field: a run of `width` bytes that holds up to `width`
characters, padded with NULs when the value is shorter, and with **no terminator when the
value fills it**. This is the one data layout `strncpy` was designed for, and it is still
everywhere binary records are: tar headers, `utmp`, ELF section names, database pages,
network protocols. The skill is to keep two ideas apart in your hands: a *field* (measured
with `strnlen`, never passed to `strlen` or `%s`) and a *string* (terminated). Every function
here crosses that boundary in one direction or the other. It comes back in S01's fixed-field
exercise, S11's `%.*s`, and the S18 bug zoo.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `field.c`, below the marked line.

## Functions

```c
int field_put(char *field, size_t width, const char *s);
size_t field_len(const char *field, size_t width);
size_t field_get(char *dst, size_t cap, const char *field, size_t width);
int field_eq(const char *field, size_t width, const char *s);
```

| Function | Contract | Example, `width` = 8 |
|---|---|---|
| `field_put` | Store the string `s`: its first `width` characters at most, then NULs up to `width`. Always writes exactly `width` bytes. Returns 1 if `s` was cut, else 0. | `"tar"` → `tar\0\0\0\0\0`, returns 0; `"abcdefghij"` → `abcdefgh`, returns 1 |
| `field_len` | The number of characters in the field, 0 to `width`. | `tar\0\0\0\0\0` → 3; `abcdefgh` → 8 |
| `field_get` | Copy the value out as a string into `dst` (`cap >= 1` bytes), cut to `cap - 1` characters if it must be. Returns the field's length, so a result `>= cap` means truncated. | field `abcdefgh`, `cap` 16 → `abcdefgh\0`, returns 8; `cap` 5 → `abcd\0`, returns 8 |
| `field_eq` | 1 if the field's value is exactly the string `s`, else 0. | field `abcdefgh` vs `"abcdefgh"` → 1; vs `"abcdefg"` → 0; vs `"abcdefghi"` → 0 |

None of the four may read or write `field[width]`. The harness gives you a field of exactly
`width` bytes, so doing it is a sanitizer report.

## Rules

- Start every rep from a fresh copy: `sh scripts/rep.sh units/s03-copying/drills/r02-fixed-field-reps`.
- No lookups during a rep.
- Any `<string.h>` function is allowed, and choosing the right ones is the drill. `strlen` on
  a field and `strcmp` against a field are the two mistakes to stop making.

## Target

12 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness uses an 8-byte field: a short value, a value of exactly 8 characters, a value
that must be cut, the empty value, a short value written over a long one (no stale
characters may remain), reads into buffers of 16, 5 and 1 bytes, and six equality cases. It
prints `s03-r02-fixed-field-reps: all checks passed`.
