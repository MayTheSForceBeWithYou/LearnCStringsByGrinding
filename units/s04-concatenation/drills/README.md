# S04 drills

Drills are short, separate repetitions of one skill. They do not build on each other, and they
never touch the exercises. Each is graded by the same harness as the exercises: the starter
fails to link until you write the functions, and `make check` must pass with ASan, UBSan and
the leak check silent.

Do every drill at least three times, on different days, from a fresh copy each time:

```bash
sh scripts/rep.sh units/s04-concatenation/drills/r01-append-reps
cd reps/s04-concatenation/r01-append-reps/01 && make check
```

`reps/` is ignored by git. Each rep gets its own numbered folder, so you can compare today's
attempt with last week's. Time yourself; the target is in each drill's README.

These three are appending, taken apart. r01 is the motion: write at a position you already
have, and keep it. r02 is the arithmetic that decides whether you may, written so that no
expression can wrap. r03 puts both into the job appending is usually for — joining a list —
with the three honest answers about truncation.

| # | Folder | Skill | Target |
|---|---|---|---|
| 1 | `r01-append-reps` | append a string, a character and a number at an end pointer | 10 min, no lookups |
| 2 | `r02-room-math-reps` | room, fits, and sizes for two and three parts, without wrapping | 10 min, no lookups |
| 3 | `r03-join-reps` | join n strings into a fixed buffer, three ways | 12 min, no lookups |
