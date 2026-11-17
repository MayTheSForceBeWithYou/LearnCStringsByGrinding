# s03-r03-stpcpy-chain-reps `[DRILL]` (XS)

## Skill

Build a string out of parts while holding a pointer to its end, so that each append costs
only the part being appended. A function that returns "where I stopped" (`stpcpy`) composes;
a function that returns "where I started" (`strcpy`, `strcat`) forces the next call to walk
everything written so far to find the end again, and a loop of those is quadratic. The
end-pointer habit is the whole of S04 (concatenation) in miniature, and it is how every
string builder, formatter and serializer you will write keeps its place.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `chain.c`, below the marked line.

## Functions

```c
char *chain_two(char *dst, const char *a, const char *b);
size_t chain_path(char *dst, const char *a, const char *b, const char *c);
char *chain_repeat(char *dst, const char *s, size_t n);
char *chain_bounded(char *dst, size_t cap, const char *const *parts, size_t count);
```

| Function | Contract | Example |
|---|---|---|
| `chain_two` | Write `a` then `b` into `dst`; the caller guarantees room. Returns a pointer to the result's terminator. | `("foo", "bar")` → `foobar`, returns `dst + 6` |
| `chain_path` | Write `a/b/c`: the three parts, copied as they are, with one `/` between them. Returns the result's length. | `("usr", "local", "bin")` → `usr/local/bin`, returns 13; `("", "", "")` → `//`, returns 2 |
| `chain_repeat` | Write `n` copies of `s`. `n == 0` gives the empty string. Returns a pointer to the terminator. | `("ab", 3)` → `ababab`, returns `dst + 6` |
| `chain_bounded` | Write the `count` strings of `parts[]` one after another into the `cap`-byte buffer `dst`, checking the room **before** writing each part. Returns a pointer to the terminator, or NULL if they do not all fit; on NULL, `dst` is the empty string when `cap > 0` and untouched when `cap == 0`. | `{"red", "", "green", "blue"}`: 13 bytes → `redgreenblue`, returns `dst + 12`; 12 bytes → NULL, `dst` = `""` |

In `chain_bounded`, twelve characters need thirteen bytes. The room check is the comparison
you must get right: how many characters still fit, keeping one byte for the terminator.

## Rules

- Start every rep from a fresh copy: `sh scripts/rep.sh units/s03-copying/drills/r03-stpcpy-chain-reps`.
- No lookups during a rep.
- **Never measure the destination.** No `strlen(dst)`, no `strcat`. You always know where the
  end is, because you put it there. Measuring a *part* (`strlen(parts[i])`) is fine.
- `stpcpy` and `memcpy` are allowed and are the natural tools.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness checks every function with empty parts and at exact-fit capacities, then calls
`chain_repeat(big, "x", 1000000)` and requires it to finish in under 1 second of CPU. With an
end pointer that is a million single-character copies. If each append first searches for the
end of the destination, it is five hundred billion byte reads, and the check fails. It prints
`s03-r03-stpcpy-chain-reps: all checks passed`.
