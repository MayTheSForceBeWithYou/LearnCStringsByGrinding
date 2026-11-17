# S01 drills

Drills are short, separate repetitions of one skill. They do not build on each other, and they
never touch the exercises. Each is graded by the same harness as the exercises: the starter
fails to link until you write the functions, and `make check` must pass with ASan, UBSan and
the leak check silent.

Do every drill at least three times, on different days, from a fresh copy each time:

```bash
sh scripts/rep.sh units/s01-length/drills/r01-strlen-reps
cd reps/s01-length/r01-strlen-reps/01 && make check
```

`reps/` is ignored by git. Each rep gets its own numbered folder, so you can compare today's
attempt with last week's. Time yourself; the target is in each drill's README.

These three cover the three things you do with a length. r01 measures — and measures *with a
bound*, which is the version that works on data from outside your program. r02 keeps the
answer as a pointer instead of a number, which is how every appender in the course holds its
place. r03 uses a one-pass loop to answer a question about the whole string, without ever
asking how long it is.

| # | Folder | Skill | Target |
|---|---|---|---|
| 1 | `r01-strlen-reps` | `strlen` three ways, plus the `len < cap` test | 8 min, no lookups |
| 2 | `r02-end-pointer-reps` | end pointers: where the string ends, how much room is left | 8 min, no lookups |
| 3 | `r03-count-reps` | four one-pass counts: bytes, words, lines, the longest run | 10 min, no lookups |
