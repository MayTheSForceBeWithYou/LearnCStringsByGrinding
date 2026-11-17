# s00-r03-escape-reps: notes

## Why it is correct

The design is one small function and three users of it:

```c
static void put(char *out, size_t cap, size_t *at, char c)
{
    if (*at + 1 < cap)
        out[*at] = c;
    (*at)++;
}
```

`put` separates *counting* from *storing*: the counter always advances, the store happens only
when there is room for this character and still a byte for the terminator. That one line is
the `snprintf` contract. Everything above it can then be written as if the buffer were
infinite, which is why `render` is a plain `switch` with no capacity arithmetic in it at all,
and why `esc_buf` is a loop with no bookkeeping.

`finish` writes the terminator at `min(at, cap - 1)` and returns the uncut count, so the
result is terminated whenever `cap > 0`, the return value reports the full length, and
`cap == 0` touches nothing.

The `switch` handles the five named bytes first, then the printable range, then falls through
to `\xHH`. Order matters for exactly one byte: `0x5C` is printable, so the backslash case has
to come before the range test, or a backslash would render as itself and the output would be
ambiguous.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| `cap == 0` | `*at + 1 < cap` is false for every `at`; `finish` writes nothing |
| a capacity that cuts mid-escape | `put` stores per character, so `\xE9` can end up as `\x` |
| the terminator byte | the `'\0'` case of the `switch`, so it renders as `\0` rather than ending anything |
| a backslash | its case precedes the printable-range test |
| `0x7F` (delete) | the range test is `<= 0x7E`, so it falls through to `\x7F` |
| bytes above 0x7F | the parameter is `unsigned char`, so no byte is ever negative |
| the nibble extraction | `b >> 4` and `b & 0x0F` on an `unsigned char` promoted to `int`: always 0 to 15 |
| `esc_str` of the empty string | `n` is 0, so it renders one byte, the terminator: `\0` |

The sign of the input is worth a second look. `render` takes an `unsigned char`, and
`esc_buf` reads through `const unsigned char *`, so `b >> 4` is 14 for `0xE9` and never -2.
The same expression on a plain `char` would print `\xFFFFFFE9` or worse, which is S00 e05's
whole subject.

## Alternatives

The library version of the whole drill is one call per byte:

```c
at += snprintf(out + at, at < cap ? cap - at : 0, "\\x%02X", b);
```

It is correct, it follows the same contract, and it is roughly twenty times slower per byte,
because every byte re-parses a format string. For a debugging tool that is usually fine; for
a hex dump of a megabyte it is not. The hand-written version is also the one you can write
inside a program that cannot call `snprintf` (a signal handler, an allocator, a sanitizer
runtime), which is where tools like this are needed most.

The other design is two passes: measure the rendering, then write it. That lets the caller
allocate exactly enough first, and it is how S11 e07's `str_escape` works, with an allocation
in between. It costs a second walk over the data and needs the two passes to agree exactly —
a class of bug this one-pass form cannot have.

A third variant worth knowing: render with a *fixed* four characters per byte
(`\xHH` for everything), which makes the output length `4 * n` and the sizing trivial, at the
cost of being unreadable. Hex dumps in debuggers do that on one side and the printable
rendering on the other.

## The usual mistake

Writing `put` as `if (*at < cap) out[*at] = c;` and terminating at the end. That stores one
character too many whenever the rendering is cut, overwriting the byte the terminator needs —
or, if the terminator is then written at `at`, overflowing the buffer. The harness's capacity
sweep (every `cap` from 0 to `need + 2`) exists to catch exactly this, and the fences around
the destination turn it into a visible failure rather than a near miss.

Second: returning the number of characters *stored* rather than needed. Then truncation is
undetectable — a full buffer and a cut one both return `cap - 1` — and the function cannot be
used to ask for a size. This is the `snprintf` return-value misunderstanding, and S03's
worked example is about the same sentence.

Third: lower-case hex, or a single digit for values below 16 (`\xE` instead of `\x0E`). Both
make the output ambiguous: `\xE9` would be indistinguishable from `\xE` followed by the
character `9`. A fixed-width escape is not decoration.

## How libc does it

There is no standard function for this, which is why every C code base ends up with one. The
nearest relatives are `snprintf("%02X")` for the hex, and `isprint()` for the range test —
and `isprint` is the one to be careful with: its answer depends on the locale, so a tool that
uses it renders different bytes differently in different environments. A debugging tool wants
the fixed ASCII range, which is what the explicit `0x20 <= b && b <= 0x7E` test gives, and
what `lib/bytes.c` does.

C has no built-in way to go the other direction either: there is no "unescape" in the library,
and `strtol` with base 16 is what you reach for when you have to parse `\xHH` back (S10). The
asymmetry is a fair summary of the C library's attitude to text: it will move your bytes and
compare them, and the meaning is yours to keep.
