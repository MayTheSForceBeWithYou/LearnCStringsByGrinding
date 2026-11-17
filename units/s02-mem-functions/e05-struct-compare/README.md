# s02-e05-struct-compare `[BUG]`

## Goal

Two records hold the same values and do not compare equal. The comparison is one line, it
uses the right function, and it is wrong. Find the defect, fix it, and explain what is in the
bytes of a struct that is not in its members. Afterwards you will never write `memcmp` on a
struct without asking the question this exercise is about.

## Background

A struct's members do not have to be adjacent. C17 6.7.2.1p15 says there may be unnamed
padding between members and after the last one, and implementations use it to put each member
at an address its type likes: on both course platforms a `char` followed by an `int` becomes
one byte, three bytes of padding, four bytes of `int` — eight bytes in all, of which three
belong to nobody.

Those three bytes are part of the object. They are copied by assignment and by `memcpy`, they
are written to a file if you write the struct to a file, and **their values are unspecified**.
A struct on the stack has whatever was in that stack slot. A struct from `malloc` has whatever
the allocator left. A struct that was zeroed and then had its members assigned has zeros. Two
structs with identical members can therefore differ in three bytes out of eight, and nothing
is wrong with either of them.

`memcmp` compares bytes. All of them. It has no idea which are members, so `memcmp(&a, &b,
sizeof a) == 0` asks "are these two objects byte-for-byte identical", which is a different
question from "do these two records hold the same values" — and answers it correctly. The
defect is in the question, not the function.

This is one of the few defect classes in this course that **no tool reports**. Reading
padding bytes is not undefined behaviour (they are part of an object you own), nothing is out
of bounds, and the comparison is a perfectly ordinary call. There is no sanitizer for "you
compared the wrong thing". The only protection is knowing the rule.

Read **Foundations: a struct has bytes that are not members** in `LESSON.md` before you start.

## Symptom

`make check` fails in the plain build, with nothing from the sanitizers. From `capture.txt`,
produced from exactly this source:

```
FAIL main.c:37: two points with the same kind and value are equal
FAIL main.c:38: and equality does not depend on the order
FAIL main.c:73: a point built elsewhere is found in the table
s02-e05-struct-compare: 3 check(s) failed
```

The first two are one case: two `Point`s with the same `kind` and the same `value`, built in
storage that held different bytes beforehand. The check just above them — that the members are
equal — passes. The last failure is the same defect in the shape it takes in a real program:
a key built in one place is not found in a table built in another.

`make asan` prints the same three lines and no report.

## What the code is supposed to do

`point.h` is the specification and it is correct.

```c
typedef struct {
    char kind;
    int value;
} Point;
```

- `point_make(kind, value)` returns a `Point` with those two members.
- `point_eq(a, b)` is **1 if the two points have the same kind and the same value**, else 0.
  Nothing else is part of a point's value.
- `point_copy(dst, src)` copies `src` into `dst`.

| Two points | `point_eq` |
|---|---|
| same `kind`, same `value`, built in differently-scribbled storage | 1 |
| same `kind`, same `value`, one of them a copy of the other | 1 |
| different `kind` | 0 |
| different `value` | 0 |
| a point and itself | 1 |

The harness builds points through a helper that scribbles a byte pattern over the storage
first and then assigns the members, so the padding differs between two otherwise identical
points. That is not a trick: it is what you get from a stack slot that held something else, a
`malloc` block, or a struct read from a network buffer.

## What to hand in

1. **The fix**, in `point.c`. There is exactly one defect, in one function, and nothing else
   in the exercise is wrong. Do not change `main.c` or `point.h`. Note that the *other* use of
   a `mem` function in the same file is correct — part of the exercise is saying why.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file, the line, the line quoted, and why the other `mem` call in the
     file is fine.
   - *The mechanism*: draw the eight bytes of a `Point` — which is `kind`, which four are
     `value`, which belong to neither — then say what the two points of the first check hold
     in each byte and what `memcmp` therefore returns. Say what the standard says about those
     bytes.
   - *How the tool showed it*: it did not. Say why each tool has nothing to report, which
     checks caught it, and why the table search is the shape this defect takes in real code.
   - *The rule you take away*: one sentence.

## Constraints

- `point_eq` keeps its signature and its meaning: same kind and same value, nothing else.
- Do not change the struct, and do not add a "zero the padding" step to `point_make`. (It
  would make the harness pass and leave every point built anywhere else still broken. Why that
  is so is worth a sentence in your answer.)
- `point_copy` stays as it is.
- No allocation.

## Verify

```bash
make check
```

Done means `s02-e05-struct-compare: all checks passed` in the plain stage and again in the
ASan stage, and `== check: OK`.

Afterwards, in a scratch file outside this directory, print `sizeof(Point)` next to
`sizeof(char) + sizeof(int)` and the bytes of a `Point` whose storage you scribbled first —
`lib/bytes.h`'s `bytes_show` does the rendering. Then reorder the members (`int` first,
`char` second) and look again. The struct is still 8 bytes, and the padding has moved to the
end.

<details><summary>Hint</summary>

Three of the eight bytes are not `kind` and are not `value`. What does the standard say their
values are?

</details>
