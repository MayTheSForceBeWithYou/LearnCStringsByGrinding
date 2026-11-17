# S03 drills

Drills are short, separate repetitions of one skill. They do not build on each other, and they
never touch the exercises. Each is graded by the same harness as the exercises: the starter
fails to link until you write the functions, and `make check` must pass with ASan, UBSan and
the leak check silent.

Do every drill at least three times, on different days, from a fresh copy each time:

```bash
sh scripts/rep.sh units/s03-copying/drills/r01-bounded-copy-reps
cd reps/s03-copying/r01-bounded-copy-reps/01 && make check
```

`reps/` is ignored by git. Each rep gets its own numbered folder, so you can compare today's
attempt with last week's. Time yourself; the target is in each drill's README.

The three drills are the three things you will type most often for the rest of the course: a
copy that respects a capacity, a field that is not a string, and an end pointer that makes
appending free.

| # | Folder | Skill | Target |
|---|---|---|---|
| 1 | `r01-bounded-copy-reps` | copy a string into `cap` bytes four ways, each reporting truncation | 12 min, no lookups |
| 2 | `r02-fixed-field-reps` | write, measure, read and compare a NUL-padded fixed-width field | 12 min, no lookups |
| 3 | `r03-stpcpy-chain-reps` | build a string from parts with an end pointer, never rescanning | 10 min, no lookups |
