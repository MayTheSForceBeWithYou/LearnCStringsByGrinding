# s01-r03-count-reps `[DRILL]` (XS)

## Skill

Four counts over a string, each in one pass, each with a different kind of state: a plain
tally, a flag for "am I inside a word", a count that has to handle the last line specially,
and a running maximum. None of them is hard; keeping them one pass and getting their edge
cases right is the drill.

This is the shape of every line-oriented and word-oriented tool you will write — `wc`, a log
filter, a CSV reader's field counter, the `collapse_spaces` of S07 e06 and the tokenizer of
S09. The habit worth building is noticing *which* state a count needs: a tally needs none, a
word count needs one bit of history, and a run length needs the previous byte.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `countreps.c`, below the marked line.

## Functions

```c
size_t count_byte(const char *s, char c);
size_t count_words(const char *s);
size_t count_lines(const char *s);
size_t longest_run(const char *s);
```

In all four, `s` is a string, is borrowed, and is not modified. The terminator is not a
character of the string and is never counted.

| Function | Definition | Examples |
|---|---|---|
| `count_byte` | how many bytes of `s` equal `c`. `c == '\0'` gives 0. A byte above 0x7F counts like any other | `count_byte("banana", 'a')` is 3; `count_byte("ab", '\0')` is 0 |
| `count_words` | a word is a run of one or more bytes that are not `' '`, `'\t'` or `'\n'`. Runs of separators and separators at either end do not make empty words | `count_words("  a  bb  ")` is 2; `count_words("")` is 0; `count_words("\n\n")` is 0 |
| `count_lines` | a line is text up to a `'\n'` or up to the terminator; a final line with no newline counts | `count_lines("")` is 0; `count_lines("\n")` is 1; `count_lines("a\nb")` is 2; `count_lines("a\n")` is 1 |
| `longest_run` | the length of the longest run of one repeated byte | `longest_run("")` is 0; `longest_run("aabbba")` is 3; `longest_run("abc")` is 1 |

The four rows of `count_lines` are the ones to get straight, and they are the behaviour `wc -l`
famously does *not* have: `wc -l` counts newlines, so it reports 0 for `"abc"` and 1 for
`"a\n"`. This function counts lines, so it reports 1 for both. Decide which you mean, in your
own code, every time.

## Rules

- Start every rep from a fresh copy:
  `sh scripts/rep.sh units/s01-length/drills/r03-count-reps`.
- No lookups during a rep: not the lesson, not `man`, not an earlier rep.
- No library call: `strlen`, `strnlen`, `memchr`, `strchr`, `strtok`, `strspn`, `strcspn` and
  `strstr` are in `FORBID`. Several of them would do a whole function here, and that is the
  point: the loops are the drill, and S09 is where the library's versions get their cards.
- **One pass each.** No measuring the string first, and no second loop.
- No allocation, no static state: two calls in a row must give the same answer.

## Target

10 minutes per rep. At least 3 reps, on different days.

## Verify

```bash
make check
```

The harness runs thirteen fixed strings — empty, one character, repeats, separators at both
ends, several newline arrangements, a tab, bytes above 0x7F — through all four functions, each
string in a buffer of exactly its own size; checks that `count_byte(s, '\0')` is 0; then
compares all four against an independent byte-by-byte count on 2000 generated strings drawn
from an alphabet that includes spaces, tabs and newlines. It prints
`s01-r03-count-reps: all checks passed`.

When a generated case fails, the harness prints the string in byte notation and all four
numbers next to what they should have been. The usual culprit is `count_lines` on input that
ends with a newline, or `longest_run` on a string whose longest run is at the very end.
