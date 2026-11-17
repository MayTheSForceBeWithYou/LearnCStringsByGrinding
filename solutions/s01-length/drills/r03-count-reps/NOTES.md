# s01-r03-count-reps: notes

## Why it is correct

Each function is one loop that ends at the terminator, and they differ only in the state they
carry.

**`count_byte`** carries nothing but the tally. The early `return 0` for `c == '\0'` states the
contract rather than relying on the loop: the loop would return 0 anyway, because it stops at
the terminator before comparing it, but a reader should not have to work that out.

**`count_words`** carries one bit: "am I inside a word". A word is counted when a non-separator
is seen while that bit is clear, which makes a word counted exactly once, at its first byte.
Runs of separators clear the bit repeatedly and count nothing, so separators at either end and
between words need no special case.

**`count_lines`** counts a line at its *last* byte, and a byte is the last of a line when it is
a newline or when the next byte is the terminator:

```c
if (s[i] == '\n' || s[i + 1] == '\0')
    n++;
```

`s[i + 1]` is safe because the loop has already established that `s[i]` is not the terminator,
so `i + 1` is at most the index of the terminator — never past it. This is the one place in the
drill where a loop looks one byte ahead, and it is worth noting why it is allowed. The two
clauses also cannot double-count: a newline that is the last byte makes the first clause true
and the second one true as well, but the `||` counts once.

**`longest_run`** carries the run length and the best seen, and compares the current byte with
the previous one:

```c
run = (i > 0 && s[i] == s[i - 1]) ? run + 1 : 1;
if (run > best)
    best = run;
```

Updating `best` inside the loop rather than at the end is what makes a run that reaches the
terminator count — the mistake the other way round loses the longest run whenever it is last.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| the empty string | every loop's condition is false at once; all four return 0 |
| `c == '\0'` | `count_byte`'s early return |
| the terminator | never compared in the body: the loop ends first |
| separators at either end | `count_words`' flag, which counts at a word's first byte |
| a string of only separators | the flag is never set with a non-separator; 0 words |
| input ending in a newline | `count_lines`' first clause counts it; no extra line follows |
| input not ending in a newline | the second clause, `s[i + 1] == '\0'` |
| a single `"\n"` | one line, by the first clause |
| the longest run at the end | `best` is updated inside the loop |
| bytes above 0x7F | every test is an equality against another byte or a literal separator |

## Alternatives

| This function | The library |
|---|---|
| `count_byte` | `memchr` in a loop over `strlen(s)` bytes, or `strchr` in a loop (S08) |
| `count_words` | `strspn`/`strcspn` alternating: skip separators, span a word, repeat (S09 e02) |
| `count_lines` | `strchr(s, '\n')` in a loop, plus the final-line test (S08) |
| `longest_run` | nothing: there is no library function for this shape |

The `strspn`/`strcspn` version of `count_words` is the one worth looking forward to, because it
is two calls in a loop and no flag at all:

```c
while (*s != '\0') {
    s += strspn(s, " \t\n");        /* skip separators */
    if (*s == '\0') break;
    n++;
    s += strcspn(s, " \t\n");       /* skip the word */
}
```

That is the scanner shape of S09, and it replaces "one bit of history" with "two primitives
that each consume a run". Both are one pass; the second is usually easier to get right, which
is why the library has those two functions at all.

For `count_lines`, the version that counts newlines instead of lines is one clause shorter and
answers a different question. `wc -l` counts newlines; a text editor counts lines. Neither is
wrong, and a program that is not sure which it means has a bug waiting for a file without a
trailing newline.

## The usual mistake

`count_lines` on input that ends with a newline, counted twice: once for the `'\n'` and once
for "the next byte is the terminator". Writing the two clauses as separate `if`s with separate
increments produces exactly that, and `"a\n"` then reports 2. The `||` is what keeps a byte
from being the last byte of two lines.

Second: `longest_run` that updates `best` after the loop instead of inside it, so a string
whose longest run is at the end reports the previous best. `"abbb"` gives 1 instead of 3. The
generated cases catch it quickly, because a run at the end is common in random strings over a
small alphabet.

Third: `count_words` that counts separators and adds one. For `"a b"` that gives 2, which is
right, and for `"  a  bb  "` it gives 7. Counting transitions, not separators, is what makes
runs of separators harmless.

Fourth, and the one that costs the most in real code: measuring first. `for (size_t i = 0; i <
strlen(s); i++)` recomputes the length on every iteration, so each of these functions becomes
quadratic. The loops here stop at the terminator and never ask how long the string is, which
is both faster and immune to e03's `strlen(s) - 1`.

## How libc does it

None of these four is in the library, and the reason is instructive: they are all *policies*
(what counts as a word, what counts as a line) rather than mechanisms. What the library
provides is the mechanisms — `memchr` to find a byte, `strspn`/`strcspn` to consume a run of a
set, `strtok_r` to split — and each of them is vectorised, so a count built on them runs
several times faster than a byte loop on long input.

The one place the library does take a position on lines is input: `fgets` stops after a
newline and keeps it, and `getline` does the same and tells you the length, so the "final line
with no newline" case is visible to the caller rather than hidden. S12 is that unit, and
`count_lines`'s two clauses are exactly the two cases a `getline` loop has to handle.
