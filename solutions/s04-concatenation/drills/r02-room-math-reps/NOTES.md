# s04-r02-room-math-reps: notes

## Why it is correct

Every one of the four follows the same discipline: **test before you compute**.

```c
size_t room_left(size_t cap, size_t len)
{
    if (cap == 0 || len + 1 >= cap)
        return 0;
    return cap - len - 1;
}
```

`len + 1 >= cap` covers a full buffer, an over-full one (`len > cap`, which a corrupt caller
can produce) and `cap == 0`, so by the time the subtraction runs, `len + 1 < cap` and the
result is positive. The `cap == 0` clause is written separately because `len + 1` could itself
wrap if `len` were `SIZE_MAX` — the condition is checked left to right, and `||` stops at the
first true clause.

```c
int will_fit(size_t cap, size_t len, size_t add)
{
    if (cap == 0 || len >= cap)
        return 0;
    return add <= cap - len - 1;
}
```

The comparison is `add <= cap - len - 1` rather than `len + add + 1 <= cap`, and that is the
whole point: the left form never adds two unbounded values. `cap - len - 1` is safe because
`len < cap` was established.

```c
size_t size_needed(size_t a, size_t b)
{
    if (b > SIZE_MAX - a) return 0;      /* a + b would wrap */
    if (a + b > SIZE_MAX - 1) return 0;  /* ... + 1 would wrap */
    return a + b + 1;
}
```

Two checks for two additions. `b > SIZE_MAX - a` is the standard overflow test for unsigned
addition: it asks the question without performing the operation.

`size_needed3` reduces to the same function after accounting for the two separators:

```c
if (a > SIZE_MAX - 2) return 0;
a += 2;
if (b > SIZE_MAX - a) return 0;
return size_needed(a + b, c);
```

so the three parts plus two separators plus the terminator is `a + 2 + b + c + 1`, with a wrap
test before each addition. Reusing `size_needed` for the last step means the terminator's `+ 1`
is written once in the file.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the first clause of both predicates |
| `cap == 1` | `len + 1 >= cap` is true at `len == 0`: room 0, and `""` fits |
| a full buffer (`len == cap - 1`) | the same test |
| `len > cap` | `len >= cap` and `len + 1 >= cap` |
| `add == SIZE_MAX` | `add <= cap - len - 1` compares, never adds |
| `a + b` wrapping | `b > SIZE_MAX - a` |
| `a + b + 1` wrapping | `a + b > SIZE_MAX - 1` |
| three parts near `SIZE_MAX/3` | the wrap tests inside `size_needed3`, which report 0 |
| the largest legal result | `size_needed(SIZE_MAX - 1, 0)` is `SIZE_MAX` |

## Alternatives

The same tests written with the additions, which is what most code contains:

```c
if (len + add + 1 <= cap) ...      /* wrong when len + add wraps */
```

For lengths that come from `strlen` of real strings this never wraps, and that is exactly why
the habit is worth drilling: the one time it matters is when a length came from a file, a
network or an attacker, and then it matters completely. A wrapped test admits a copy that
destroys the heap.

C23 adds checked arithmetic (`ckd_add` and friends in `<stdckdint.h>`), which performs the
operation and tells you whether it overflowed:

```c
size_t t;
if (ckd_add(&t, a, b)) return 0;
```

That is the better interface and it is not available in C17, which this course compiles as.
The `b > SIZE_MAX - a` form is the portable equivalent and is worth having in your fingers.

A third option is to bound the inputs instead of the arithmetic: refuse any length above some
sane maximum (`1 << 30`, say) at the point where it enters the program, and then ordinary
addition is safe everywhere. That is a real design, used in parsers and protocol code, and it
moves the check from a hundred expressions to one.

## The usual mistake

`cap - len - 1` computed before `len < cap` is known. With `len == cap` that is `SIZE_MAX`,
and every subsequent "does it fit" test says yes. This is the same defect as S01 e03's
`strlen(s) - 1`, in the place where it does the most damage.

Second: `len + add + 1 <= cap` as the fits-test. Correct for every input a test suite will
produce, and wrong for the one an attacker will.

Third: reporting an impossible size as `SIZE_MAX` rather than 0. `SIZE_MAX` is a plausible
size that a caller might pass to `malloc` (which will fail, so the damage is limited) — but it
is indistinguishable from a real answer, while 0 cannot be a string's size and makes a sloppy
caller's `malloc(0)` fail loudly rather than quietly.

Fourth: writing `size_needed3` as `size_needed(size_needed(a, b), c)`. It is tempting and it
counts one separator too few: `size_needed` already includes the terminator, so composing it
twice adds a terminator where a separator was needed. The arithmetic has to be done once, with
the separators explicit.

## How libc does it

Nothing in the C library does this arithmetic for you, and that is the reason the family of
bounded functions is shaped the way it is: `snprintf`, `strlcat` and `strnlen` all take a
`size` and work out the room internally, so that the caller's only job is to pass
`sizeof buf` — the one expression that cannot be wrong.

Where the library does expose the question is in allocation. `calloc(nmemb, size)` multiplies
for you **and checks for overflow**, which is precisely why it is preferred over
`malloc(n * size)`: the multiplication in the second form can wrap and produce a block far
smaller than the caller believes. The habit in this drill — test the arithmetic, not the
result — is the same one, and `calloc` is the library admitting that callers get it wrong.
