# s04-e03-shlemiel: notes

## Why it is correct

The fix writes at the offset the function is already tracking:

```c
if (need + wlen < cap) {
    memcpy(dst + need, words[i], wlen);
    dst[need + wlen] = '\0';
}
need += wlen;
```

`need` is the length of the string so far, which is where the terminator is, which is where
the next piece goes. Nothing has to be searched for. The separator is written the same way,
one byte at `dst[need]` followed by a terminator.

The contract is unchanged: `need` still accumulates the *wanted* length whether or not the
piece fitted, so the return value is the same as before and the truncation rule
("a piece is written only if it fits entirely") is the same `need + wlen < cap` test.

Each word is now read once (by `strlen`) and copied once, so the whole join is
`O(total output)`. On the harness's case that is 0.0004 s against 0.14 s — the same factor the
lesson's demo table shows growing as `n` doubles.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | `if (cap > 0) dst[0] = '\0';` and every `need + ... < cap` test, which is false |
| `n == 0` | the loop never runs; the buffer is the empty string and 0 comes back |
| one word | `i > 0` is false, so no separator |
| a word that does not fit | `need + wlen < cap` is false; `need` still grows |
| everything after it | `need` only grows, so every later test is false too |
| an exact fit | `need + wlen < cap` is true when `need + wlen == cap - 1` |
| the terminator | written after every piece that fitted, at `dst[need + wlen]` |

## Alternatives

The same loop with an end pointer instead of an offset:

```c
char *p = dst, *end = dst + cap;
...
p = app_str(p, end, words[i]);
```

which is drill r01 of this unit, and the shape S04 e05's cursor packages. Offsets and pointers
are equivalent here; the offset version is slightly easier when the same number is also the
return value, as it is in this function.

With the library:

```c
int k = snprintf(dst + need, cap > need ? cap - need : 0, "%s%s", i ? " " : "", words[i]);
```

Correct, linear, and it parses a format string per word — about twenty times the cost per
call of the `memcpy` version, which for a join of 60,000 words is the difference between
0.0004 s and 0.01 s. `snprintf` is the right tool when the piece needs formatting and the
wrong one when it is already a string.

What is *not* an alternative is keeping `strcat` and adding a length cache outside the
function: the moment two places track the same end position, they drift.

## The usual mistake

Writing the loop with `strcat` in the first place. It reads well, it is what the obvious
mental model of "append" suggests, and it is quadratic. The tell is that the function already
has the information `strcat` is about to recompute — which is true in almost every real
appending loop, because the code needs the length for something else anyway.

Second: "fixing" it with a length cache that is recomputed from `strlen(dst)` once per
iteration. That is the same quadratic cost with more code.

Third: fixing it only for large `n` ("use `strcat` for a few words, `memcpy` beyond ten").
Two code paths, one of them rarely exercised, for no benefit: the linear version is simpler
*and* faster at every size.

Fourth: changing the contract while fixing the shape — dropping the `need` accumulation and
returning the stored length instead. Then truncation becomes undetectable, which is the S03
`snprintf` lesson undone.

## How libc does it

Nothing in libc can fix this, and it is worth being precise about why. glibc's `strcat` is
`strlen` plus `memcpy`, both vectorised: the walk runs 32 or 64 bytes per step, so it is
perhaps fifty times faster than a byte loop. That changes the constant and not the exponent.
Doubling `n` still quadruples the work, which is exactly what the lesson's table shows as the
ratio climbs from 47× to 475× over four doublings.

The interface is the problem: a function told only "here is the start of a string" must find
the end. Every library that has tried to fix it has done so by changing the interface rather
than the implementation — `stpcpy` and `stpncpy` return the end (S03), `snprintf` returns the
length, `strlcat` returns the wanted length, and the Linux `string_copying(7)` page's
`stpecpy` sketch takes an end pointer. The common thread, and the point of this unit, is that
an appending interface must either take or return the position, or it will be used in a loop
and the loop will be quadratic.
