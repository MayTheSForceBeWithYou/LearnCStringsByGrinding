# s04-e07-path-join: notes

## Why it is correct

The whole rule lives in one function that either counts or writes:

```c
static size_t build(char *out, const char *const *parts, size_t n);
```

With `out == NULL` nothing is written and the return value is the length needed; with a real
pointer the result is written and the same number comes back. The two passes cannot disagree
about the separator rule, because they are the *same code* — which is the point of the shape,
not a convenience. `path_join_n` is then four lines: count, allocate `total + 1`, write,
return.

Inside `build`, each part is reduced to two numbers:

```c
size_t b = trimmed_begin(parts[i]);   /* leading slashes */
size_t e = trimmed_end(parts[i]);     /* length without trailing slashes */
size_t lead = (i == 0) ? b : 0;       /* only the first part keeps them */
size_t body = e > b ? e - b : 0;
```

and the separator decision is one flag:

```c
if (any && !ends_slash) { emit '/'; }
```

"Something has been written and it did not end in a slash." That covers all three awkward
cases at once: `"/" + "bin"` has `ends_slash` set by the first part's lead, so no separator is
added; `"" + "bin"` has `any` clear, so none either; and `"a" + "b"` has both conditions true,
so one is.

`path_join` builds a two-element array and calls `path_join_n`, so there is one implementation
of the rule in the file and one place for a bug to live.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| a part that is entirely slashes, first | `lead = b` with `body == 0`: the slashes are kept |
| a part that is entirely slashes, later | `lead = 0`, `body = 0`: it contributes nothing |
| an empty part | the same: nothing, and no separator |
| `n == 0` | the loop never runs; `total` is 0 and the result is `""` |
| doubled slashes on either side | `trimmed_begin`/`trimmed_end` collapse them |
| an absolute first part | `lead` for `i == 0` only |
| the terminator | `out[at] = '\0'` after the loop, inside the `out != NULL` guard |
| allocation failure | `malloc` returns NULL and the function returns NULL, having allocated nothing else |

## Alternatives

A single pass that allocates an upper bound (`sum of lengths + n + 1`) and then shrinks with
`realloc`:

```c
char *out = malloc(upper);
... write ...
char *tmp = realloc(out, at + 1);
if (tmp != NULL) out = tmp;
```

One pass over the parts instead of two, at the cost of an allocation that is bigger than
needed and a `realloc` that may move the block. It is a reasonable trade when measuring is
expensive (parts arriving from a generator, say); here measuring is a `strlen` per part, so
the two-pass version is simpler and allocates exactly once — which is what the harness checks.

With `open_memstream` (S13) the whole function becomes `fprintf` into a growable stream, with
the library doing the sizing. That is the right answer in a program that already uses streams,
and it has a property worth noting for a `[FAULT]` exercise: the allocations happen *inside
libc*, where `FAIL_AT` cannot reach them (`docs/AUTHORING-INFRA.md`), so the fault sweep can
no longer prove anything about them.

For the separator rule itself, the alternative design is to normalise each part first (into a
scratch buffer) and then join unconditionally. It reads more simply and needs either a second
allocation per part or a fixed-size buffer — the first costs the all-or-nothing property real
work, the second breaks on a long part.

## The usual mistake

Measuring with one rule and writing with another. It is the defect this shape exists to
prevent, and it is always the same symptom: a buffer one or two bytes too small, found as a
`heap-buffer-overflow WRITE` at the terminator, or a string with a stray separator. Two copies
of three lines of arithmetic drift apart the first time the rule changes.

Second: `malloc(total)` instead of `malloc(total + 1)`. The classic off-by-one, and it is
invisible until the terminator is written — S01 e05 is a whole exercise about reading that
report backwards.

Third: handling `"/" + "bin"` by collapsing *all* slashes, which turns an absolute path into a
relative one. A path joiner that does that is a security bug in the making: `/etc/passwd` and
`etc/passwd` are different files, and the second is relative to wherever the program happens
to be.

Fourth, specific to `[FAULT]` exercises: allocating, then discovering a problem, and returning
NULL without freeing. With one allocation it is hard to get wrong; the sweep exists because
with two it is easy, and because the habit is what carries into S10 e07 and S13 e07.

## How libc does it

There is no path joiner in ISO C or POSIX. `basename` and `dirname` (POSIX) take paths apart
and are notorious for modifying their arguments and returning pointers to static storage;
`realpath` resolves one but requires the file to exist. Every program that builds paths writes
this function, which is why it is worth writing once with the cases enumerated.

The closest thing to a standard answer is `snprintf(buf, cap, "%s/%s", dir, name)` with the
truncation checked — correct, bounded, and it produces `usr//bin` when `dir` already ends in a
slash, because it knows nothing about paths. Most programs decide that double slashes are
harmless on POSIX (they are, except at the start, where `//` is implementation-defined) and
stop there. This exercise does the tidy version because the trimming is where the interesting
`size_t` arithmetic is.
