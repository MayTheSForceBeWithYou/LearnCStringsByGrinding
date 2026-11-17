# s04-r03-join-reps: notes

## Why it is correct

One helper carries the `snprintf` contract for both writers:

```c
static void put(char *dst, size_t cap, size_t *at, const char *s, size_t slen)
{
    if (cap > 0 && *at < cap - 1) {
        size_t room = cap - 1 - *at;
        size_t keep = slen < room ? slen : room;

        memcpy(dst + *at, s, keep);
        dst[*at + keep] = '\0';
    }
    *at += slen;
}
```

`*at` always advances by the full `slen`, whether or not anything was stored — that is what
makes the final `at` the *wanted* length and the return value useful. The store happens only
when there is room, and `keep` reserves the terminator's byte, so `dst[*at + keep]` is always
inside the buffer. `cap == 0` writes nothing and never touches `dst`.

`join_into` is then the loop with no bookkeeping of its own:

```c
if (cap > 0) dst[0] = '\0';
for (i = 0; i < n; i++) {
    if (i > 0) put(dst, cap, &at, sep, slen);
    put(dst, cap, &at, parts[i], strlen(parts[i]));
}
return at;
```

The separator goes before every part except the first, which is what produces `a--b` for an
empty middle part: the separators are a property of the *positions*, not of the parts'
contents.

`join_prefix` asks before each part whether the part **and** its separator fit whole:

```c
size_t add = strlen(parts[i]) + (i > 0 ? slen : 0);
if (cap == 0 || add > cap - 1 - at)
    break;
```

`cap - 1 - at` is the room, and it is computed only after `cap == 0` has been ruled out; `at`
is never greater than `cap - 1` here, because the loop stops the first time something does not
fit. The count is reported through `*fitted` when it is not NULL, and the return value is
`at`, the length actually written — which, unlike `join_into`'s, is always less than `cap`.

`join_len` is the same arithmetic with no buffer at all, which is what a heap version would
call before allocating.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | the `cap > 0` test in `put` and the explicit test in `join_prefix` |
| `cap == 1` | `*at < cap - 1` is false at once: only the terminator, written by `dst[0] = '\0'` |
| `n == 0` | the loop never runs; 0 comes back and the buffer is `""` |
| one part | `i > 0` is false, so no separator |
| an empty part | `strlen` is 0; `put` advances nothing but the separators still appear |
| an empty separator | `slen` is 0; `put` is a no-op for it |
| a part cut in half | `keep == room` in `put`, for `join_into` only |
| a part that would be cut | `join_prefix` breaks before writing it |
| `fitted == NULL` | the `if (fitted != NULL)` guard |
| the wanted length | `*at += slen` outside the `if` |

## Alternatives

The heap version is `join_len` followed by one allocation and one `join_into`:

```c
size_t need = join_len(parts, n, sep);
char *out = malloc(need + 1);
if (out == NULL) return NULL;
join_into(out, need + 1, parts, n, sep);
```

which is exactly the measure-then-build shape of e07, and the reason `join_len` is a separate
function rather than a private detail.

With the library, each `put` is `snprintf(dst + at, cap > at ? cap - at : 0, "%s", s)` and the
return value is added to `at`. Correct, and it parses a format string per piece; for a join of
many short parts that is the dominant cost. `snprintf` earns its keep when a piece needs
formatting (a number, a padded field), not when it is already a string.

`join_prefix`'s policy — whole parts only — is the third of S03 e06's three policies
(truncate, refuse, all-or-nothing) applied per element. Real code picks it when a half-written
element would be misread: a list of file names, a CSV row, a set of flags. For prose, cutting
mid-word is fine and `join_into` is the right choice.

## The usual mistake

Stopping the loop in `join_into` as soon as something does not fit. The buffer ends up right
and the return value ends up too small, so the caller cannot tell how much room it needed —
and `ret >= cap` may even be false, reporting success. The contract is to keep counting: the
`*at += slen` outside the `if` is the line that does it.

Second: `join_prefix` forgetting the separator in its room test, so that the last part fits by
exactly the separator's width and then truncates. The test has to be on the part *plus* what
precedes it.

Third: rescanning the destination (`strlen(dst)` once per part) to find where to write. It
works, it is the quadratic shape of e03, and in a join of ten thousand parts it is the whole
cost of the program.

Fourth: computing `cap - 1 - at` before checking `cap > 0` and `at < cap - 1`. With `cap == 0`
that is `SIZE_MAX - at`, and `memcpy` is then handed a length that destroys the heap. Drill
r02 is the arithmetic on its own, for this reason.

## How libc does it

There is no join in the C library — it is a policy question (what separator, what happens to
empty parts, what happens on truncation) and C leaves policies to the caller. What the library
provides is the `snprintf` contract that these three functions copy, and it is worth noticing
how much of the design comes from one decision: returning the *wanted* length rather than the
stored one. That single choice makes truncation detectable, makes a sizing pass possible
(`snprintf(NULL, 0, ...)` returns the length needed), and lets a caller keep counting after
the buffer is full.

Other languages' join functions allocate, which removes the question; in C the caller owns the
buffer, so the function's only honest option is to report. The three functions here are the
three honest reports: how much would it be, how much did I want, and how much actually fitted.
