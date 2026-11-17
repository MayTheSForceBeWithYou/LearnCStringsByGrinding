# s04-r03-join-reps `[DRILL]` (S)

## Skill

Joining `n` strings with a separator into a fixed buffer, three ways: measure without writing,
write with truncation reported, and write only the whole parts that fit. The first is the
sizing pass every heap version needs; the second is the `snprintf` contract applied to a
loop; the third is what a program does when a half-written part would be worse than a short
list.

Joining is the single most common appending job — path components, CSV fields, SQL columns,
log tags — and it is where the quadratic append of e03 shows up most often in real code.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `join.c`, below the marked line.

## Functions

```c
size_t join_len(const char *const *parts, size_t n, const char *sep);
size_t join_into(char *dst, size_t cap, const char *const *parts, size_t n, const char *sep);
size_t join_prefix(char *dst, size_t cap, const char *const *p, size_t n, const char *sep, size_t *fitted);
```

All three follow the `snprintf` contract: write what fits, always terminate when `cap > 0`,
and return the length that was **wanted**, so `ret >= cap` means truncated and `ret + 1` is
the size that would have been enough.

| Function | Does | Examples with `{"alpha","beta","gamma"}` |
|---|---|---|
| `join_len` | the length, no buffer | with `"/"`, 16; with `", "`, 18; `n == 0`, 0 |
| `join_into` | writes, truncating by characters | `cap` 8 gives `alpha/b`, returns 16 |
| `join_prefix` | writes only whole parts, reports how many through `*fitted` | `cap` 16 gives `alpha/beta` and `*fitted == 2` |

Three details the harness holds you to. `join_into` truncates **by characters**, so a part may
be cut in half; `join_prefix` never cuts one, and its return value is the length of what it
actually wrote (which always fits). An empty part contributes nothing but still gets its
separators — `{"a","","b"}` with `"-"` is `a--b`. And `fitted` may be NULL, in which case the
count is simply not reported.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s04-concatenation/drills/r03-join-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- `strcat`, `strncat`, `strlcat`, `strlcpy`, `snprintf` and `sprintf` are in `FORBID`.
  `strlen` and `memcpy` are the tools.
- **No rescanning the destination**: no `strlen(dst)` anywhere, at any point in the loop.
- `cap` may be 0, and then nothing is written and nothing is read from `dst`.
- `join_into` and `join_prefix` may share a helper; `join_prefix` may use `join_len` on the
  prefix it chose.

## Target

12 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness checks `join_len` on seven cases including empty parts, an empty separator and
`n == 0`; `join_into` at twelve capacities from 32 down to 0, including a cut in the middle of
a part, a cut exactly at a separator, and a two-character separator; `join_prefix` at nine
capacities, including ones where two parts fit, where one fits, and where not even the first
does; and the return values of both. Every buffer is exactly `cap` bytes with fences around
it. It prints `s04-r03-join-reps: all checks passed`.

The two failures to expect: a `join_into` that stops appending once something did not fit
(the contract is to keep counting and keep writing what fits), and a `join_prefix` whose
`fitted` is one too many because the separator before a part was not counted against the room.
