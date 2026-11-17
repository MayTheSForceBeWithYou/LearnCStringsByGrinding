# s02-e04-memset-args `[BUG]`

## Goal

A table of fixed-size records is cleared and reused. The clearing function compiles without a
word from anybody and does nothing at all, so every record carries the previous record's bytes
into its next life. Find the defect, fix it, and explain why the compiler, which does catch
this mistake when it can see it, says nothing here. Afterwards you know `memset`'s parameter
order for good, and you know what a wrapper function hides from a diagnostic.

## Background

`memset(void *s, int c, size_t n)` takes the pointer, then the **byte value**, then the
**count**. Two of those three parameters are integers, so swapping them compiles silently, and
the result is `memset(p, size, 0)`: fill zero bytes with the value `size`. Nothing is written,
nothing is out of bounds, and the function returns successfully.

The order is worth a sentence of history, because it is the one people get wrong. `memset`
sets *to* a value, so it reads as "set this area to this byte, this many times" — pointer,
value, count. `memcpy` and `memmove` are destination, source, count. `fread` and `fwrite`, in
the same library, take the size *before* the count. There is no single rule; there is the
manual page.

Compilers do try to help. gcc and clang both have `-Wmemset-transposed-args`, which is on
under `-Wall`, and both fire when the count argument is a literal zero in plain sight. They
cannot fire here, because the call is inside a small helper whose arguments arrive as
parameters:

```c
static void fill_bytes(void *p, size_t size, int value)
{
    memset(p, size, value);          /* the compiler sees two variables */
}
```

That is not a contrived hiding place. It is what every code base does: one wrapper that adds
logging, or takes the size from a struct, or exists so the call site reads better. A
diagnostic that depends on seeing a constant disappears the moment the value is passed in,
which is why the lesson's rule is **a warning is evidence of a defect; no warning is evidence
of nothing.**

Read **Foundations: one byte, n times** and the `memset` card in `LESSON.md` before you start.

## Symptom

`make check` fails in the plain build, with nothing from the sanitizers and nothing from the
compiler. From `capture.txt`, produced from exactly this source:

```
FAIL main.c:30: slot_clear zeroes every byte of the slot
FAIL main.c:31: a cleared slot has id 0
FAIL main.c:32: a cleared slot has an empty name
       got:  ZZZZZZZZZZZZ\0
       want: \0
FAIL main.c:38: slot_clear_all zeroes every byte of every slot
```

`ZZZZZZZZZZZZ` is the `0x5A` the harness scribbled over the storage before clearing it: the
slot still holds every byte it held before. The `want: \0` is the empty name a cleared slot is
supposed to have.

`make asan` prints the same failures and no report. Nothing is out of bounds — nothing is
written at all.

## What the code is supposed to do

`slot.h` is the specification and it is correct.

- A `Slot` is an `int id` and a `char name[12]`, and the table reuses one array of them.
- `slot_clear(s)` makes **every byte** of the slot 0 — the id, the name, the bytes after the
  name's terminator, and the padding between the members. After it, the id is 0 and the name
  is the empty string, with nothing left of the record that was there.
- `slot_clear_all(table, n)` does that to `n` slots.
- `slot_set(s, id, name)` fills a slot, returning -1 and changing nothing if the name does not
  fit in `SLOT_NAME - 1` characters.

| Step | The slot afterwards |
|---|---|
| storage scribbled with `0x5A` | every byte `0x5A` |
| `slot_clear(s)` | every byte 0: `id` 0, `name` the empty string |
| `slot_set(s, 7, "ada")` | `id` 7, `name` `ada\0`, the remaining eight bytes still 0 |
| `slot_set(s, 8, "bartholomew!")` (12 characters) | unchanged, returns -1 |
| `slot_clear(s)` again | every byte 0 again |
| `slot_set(s, 10, "al")` | `name` is `al\0` and the nine bytes after it are 0 |

The last row is why "every byte" is in the contract rather than "the members": a record that
holds `maximilien` and is then reused for `al` must not have `milien` sitting after the
terminator, where the next program to write the struct to a file or hash its bytes will find
it.

## What to hand in

1. **The fix**, in `slot.c`. There is exactly one defect, in one function, and nothing else in
   the exercise is wrong. Do not change `main.c` or `slot.h`, and do not remove the helper:
   the wrapper is part of the lesson, not part of the bug.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted, and `memset`'s parameter list in order
     beside it.
   - *The mechanism*: which byte value and which count the call actually asks for, what is in
     the slot afterwards, and why each of the first three checks fails the way it does. Then:
     what would have happened if the two arguments had been 12 and 65 rather than 24 and 0 —
     that is, why a transposition is not always a no-op.
   - *How the tool showed it*: why no sanitizer reports anything; what gcc and clang print
     when the same swap is written with the arguments in plain sight (the lesson's *What the
     tools say* has both, with their flag); and why neither says anything about this file.
   - *The rule you take away*: one sentence.

## Constraints

- `slot_clear` must still clear **every** byte, padding included, not just the two members.
- `slot_clear_all` keeps its shape: it calls `slot_clear` once per slot.
- No allocation, and no change to the struct.
- Do not silence anything with a cast.

## Verify

```bash
make check
```

Done means `s02-e04-memset-args: all checks passed` in the plain stage and again in the ASan
stage, and `== check: OK`.

Afterwards, in a scratch file outside this directory, write the same swapped call with the
constants in plain sight — `memset(&s, sizeof s, 0);` — and compile it with `-Wall`. Both
course compilers catch that one. Then move it behind a two-line helper and watch the warning
disappear. The distance between those two programs is the distance between a defect a tool
finds and a defect a test finds.

<details><summary>Hint</summary>

Read the `memset` card's prototype, then read the call. Nothing else in the file is wrong.

</details>
