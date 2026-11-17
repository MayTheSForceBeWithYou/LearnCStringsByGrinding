# s00-e03-literal-write: notes

## Why it is correct

The fixed `prefix_write` builds the prefix in `out`, which is the caller's buffer and
therefore writable, and upper-cases it there:

```c
for (size_t i = 0; i < len; i++)
    out[i] = name[i];
out[len] = ':';
out[len + 1] = ' ';
out[len + 2] = '\0';
upcase(out);
```

Every byte `upcase` touches now belongs to the caller. The module's own literals are never
written, so `prefix_name` keeps returning lower-case names for the whole run, which is what
its header promises. `level_name` returns `const char *` after the fix, so the compiler
rejects any future attempt to write through it: the mistake cannot come back silently.

Calling `upcase(out)` on the whole prefix rather than on the name alone is safe because
`':'` and `' '` are not lower-case letters, and `upcase` leaves every byte that is not one
alone. The order matters, though: the copy must happen first, and the length test must happen
before the copy, so that a prefix that does not fit leaves `out` untouched.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| a buffer of the exact size | `len + 3 > cap` is false when `cap == len + 3` |
| a buffer one byte too small | `len + 3 > cap` returns -1 before any write |
| `cap == 0` | the same test: `len + 3 > 0` is true for every level |
| an unknown level | `level_name`'s `default:` gives `"error"` |
| the module's literals | never written: `name` is `const char *` and only read |
| bytes above 0x7F in a name | there are none, and `upcase` would leave them alone anyway |

## Alternatives

Copy into a local buffer, upper-case that, then copy out:

```c
char tmp[PREFIX_CAP];
/* copy name into tmp, upcase(tmp), then copy tmp into out */
```

That needs a second capacity check and a decision about what `PREFIX_CAP` should be, which is
why building in `out` is better here: there is exactly one buffer, and its size came from the
caller.

With the library, the whole function is one call once you have reached S11:
`snprintf(out, cap, "%s: ", name)` plus `upcase(out)`, with the return value checked for
truncation — and that version never needs to know `len + 3`. S03 and S04 build up to it.

A third design avoids the problem rather than fixing it: store the names in capitals in the
first place (`"WARNING"`) and let `prefix_name` lower-case them into a caller's buffer. The
rule stays the same; only which side pays for the copy changes.

## The usual mistake

"Fixing" `upcase`: making it allocate and return a new string, or having it check whether
its argument is writable. The first changes the function's contract (and gives every caller a
buffer to free); the second is not possible in portable C — there is no way to ask whether a
pointer addresses writable storage, which is exactly why the rule has to be a convention
about types.

The other common move is to make `level_name` return a `static char` array instead of a
literal, so that the write succeeds. Now the program does not crash, and it is more broken
than before: `prefix_name` returns a different string after the first `prefix_write`, which
silently breaks every other reader of that name. The checks at the end of `main.c` catch it.

## How libc does it

Nothing in libc is involved in the crash, which is part of the lesson: this is the linker and
the loader, not the library. The compiler emits the literal into a read-only section, the
linker merges identical literals from every translation unit into one copy (`"error"` appears
once however many files mention it), and the loader maps that section without write
permission. String literals being shared is why one write can corrupt an unrelated module's
data, and why implementations are keen to put them where a write cannot happen.

The one libc connection worth knowing: functions that return a pointer to storage they own
(`getenv`, `strerror`, `localeconv`, `asctime`) all carry the same rule in their manual pages
— read it, do not write it, do not free it, and copy it if you need it to last. S14 is built
on that, and `strtok` on `getenv`'s result is its classic violation (S14 e06).
