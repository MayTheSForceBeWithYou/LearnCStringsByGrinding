# Authoring rules

For everyone who writes lessons, exercises, drills or solutions in this course. Derived from
`LearnCDataStructuresByGrinding/docs/AUTHORING.md`, with the changes this course makes:
library first, descriptive reading, oracle tests, graded predictions. `docs/SPEC.md` wins any
disagreement. Build mechanics are in `docs/AUTHORING-INFRA.md`. `units/s03-copying/` is the
exemplar for everything below.

## 1. The non-negotiables

1. **Every `[WRITE]`, `[DRILL]`, `[FAULT]` and `[DESIGN]` starter fails to link until the
   student writes the real code**, on exactly the functions in `EXPECT_SYMBOLS`. Declare,
   call from the harness, define nothing. No stubs, no `#error`, no stop lines.
2. **Solutions live in `solutions/`**, mirror the exercise path, are complete programs, pass
   `make check` with gcc and clang on Linux and Apple clang on macOS, and keep the shipped
   harness byte-identical. No README links to them.
3. **Descriptive, not prescriptive.** Describe behaviour, the library, the reason, the
   checks and what failure looks like, fully. Never describe the implementation: no step
   lists, no pseudocode of the solution, at most one hint and never the algorithm.
4. **A card before a call.** No exercise requires a library function whose reference card is
   in a later unit (`docs/SPEC.md` section 6).
5. **No hidden requirements.** If the harness checks it, the README says it. A reviewer who
   reads only the README can predict every check in `main.c`.
6. **Never invent tool output**, including harness output. Every FAIL line, compiler warning
   and sanitizer report in a lesson or README is pasted from a real run of the shipped
   source; reports and warnings come from a `capture*.txt`.
7. **No shortcuts.** Never tell a student to copy code from an earlier exercise. If a later
   exercise needs an earlier function, the student writes it again or the starter provides a
   different API.
8. **Portable, and no build artifacts.** `docs/SPEC.md` section 10; `git status` stays clean.

## 2. LESSON.md

Format in `docs/SPEC.md` 8.1; 250-550 lines; `make lint` checks the sections, the length,
the cards and the tables.

- **Organizer.** What the unit is for and what changes in the reader's thinking. Then
  prerequisites by unit id and 4-6 concrete abilities.
- **The functions in this unit.** One row per function; the Card column says `below` or the
  unit that holds the card.
- **Foundations.** The mechanism before the API: which bytes, read or written, in what
  order. Put every contrast in a table with every cell filled. Name costs: bytes read, bytes
  written, passes.
- **Function reference.** One card per function, with the fixed rows of `docs/SPEC.md` 8.2
  (two rows may share a line as "Requires / Undefined if" when they are identical to another
  card's and say so). After the table, the function's rows of the byte diagram.
- **Worked examples (2-3).** A neighbouring problem, never an exercise. Work it to the
  bytes: the call, the buffer before, the buffer after, the return value, what the next line
  may assume. Check every number by running it. Each has a paragraph starting
  **The plausible wrong reading.**
- **What the tools say.** The compiler warning with its flag, the sanitizer report's first
  lines, and the recognition rule: which token on which line names what. Also what the tools
  do *not* say.
- **Distinctions, `man` pointers, Check yourself (6-10 production questions), Takeaways
  (4-7 claims that could be wrong), Exercises and Drills tables.**

Prose: one term per concept; second person, present tense; no filler. Say what the standard
says and cite the clause; keep *undefined*, *unspecified*, *implementation-defined* and
"what glibc happens to do" apart. One sentence of history when it explains a design.

## 3. Exercise READMEs

Format in `docs/SPEC.md` 8.4; `make lint` checks the sections and their order.

- **Goal**: what is built, and what the student can do afterwards.
- **Background**: what the function or idiom is, where real code uses it, which defect this
  exercise prevents. Points into the lesson by section name.
- **The API**: the prototypes exactly as in the header (lint compares them), then each
  function's contract in prose: borrowed or owned, what is written, the return value, every
  error case, what is unchanged on failure.
- **Behaviour**: at least eight concrete cases in a table, in byte notation, at least half of
  them edge cases (empty, `n == 0`, exact fit, one too many, bytes above 0x7F, no terminator).
- **Constraints**: testable. The `FORBID` list with its reason.
- **What the harness checks**: in words, one bullet per group of checks.
- **Verify**: the command, the success line, one real FAIL sample and how to read it, and
  what a sanitizer report means in this exercise.

`[BUG]`: Symptom (from a capture), What the code is supposed to do, What to hand in (the fix
and `ANSWER.md` under its four headings). State the number of defects and that there are no
others; a second author confirms. `[PREDICT]` and `[LOOKUP]`: explain the setup, the answer
syntax, and give two worked answers that are not graded. `[DESIGN]`: the questions the
contract must answer, and which choices are fixed.

## 4. Drills

Format in `docs/SPEC.md` 8.5. One skill, 3-5 small functions, 10-25 minutes. The README says
where the skill returns, gives each function's contract in one line with an example, the
rules, the time target and three reps. No hints. Self-contained.

## 5. Harnesses

- `check()`, `check_str()`, `check_bytes()`, `check_guard()`; finish with
  `return checks_done("<full id>");`. Messages say what was expected.
- **Every destination is a `guard_new(cap)` buffer of exactly the stated capacity**, and
  whole buffers are compared with `check_bytes`, so an unwritten byte, an extra terminator
  and a write past the end are all visible.
- **Every source whose end matters is a `guard_copy`** of exactly the bytes the function may
  read.
- **Reimplementations are compared with libc** on a full grid of small cases and on
  generated strings (`lib/oracle.h`), including bytes above 0x7F. Compare exactly what the
  standard fixes: values, signs only for comparisons, pointers as offsets.
- Complexity claims are tested with a CPU budget a correct solution beats by a factor of 10
  under ASan, and that the wrong shape misses by a wide margin. Measure both before
  shipping.
- Input is a string in the harness (`fmemopen`), never a file or the terminal.

## 6. Solutions

Every starter file, completed, plus `NOTES.md` with five parts (`docs/SPEC.md` 8.9): why it
is correct, edge cases and which line handles each, alternatives (including the library call
that replaces it), the usual mistake, how libc does it. `[BUG]` solutions also ship a filled
`ANSWER.md`; `[PREDICT]`/`[LOOKUP]` solutions a filled `answers.txt`; `[DESIGN]` solutions a
header with reference contracts.

## 7. Before you open a PR

- [ ] Names match `docs/SPEC.md` section 5; only paths you own
- [ ] Lesson: all sections, 250-550 lines, a card for every function, tables match folders
- [ ] READMEs: all sections, prototypes equal the header's, every check described
- [ ] Every pasted output is real; every `capture*.txt` is fresh
- [ ] A wrong implementation of each `[WRITE]` was tried and is rejected
- [ ] `make check` at the root passes with `CC=cc` and `CC=clang`; no build artifacts
