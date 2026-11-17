# S02 drills

Drills are short, separate repetitions of one skill. They do not build on each other, and they
never touch the exercises. Each is graded by the same harness as the exercises: the starter
fails to link until you write the functions, and `make check` must pass with ASan, UBSan and
the leak check silent.

Do every drill at least three times, on different days, from a fresh copy each time:

```bash
sh scripts/rep.sh units/s02-mem-functions/drills/r01-gap-reps
cd reps/s02-mem-functions/r01-gap-reps/01 && make check
```

`reps/` is ignored by git. Each rep gets its own numbered folder, so you can compare today's
attempt with last week's. Time yourself; the target is in each drill's README.

These three are the byte-level motions the rest of the course is built on. r01 moves bytes
inside one buffer, which is the operation every in-place edit is made of and the one people
reach for `memcpy` for. r02 clears and compares ranges, including the `sizeof *p` habit and
the padding that makes byte-wise struct comparison wrong. r03 is all five `mem` functions from
a blank file against the clock, which is the floor of capstone C1.

| # | Folder | Skill | Target |
|---|---|---|---|
| 1 | `r01-gap-reps` | open, close, delete, insert: `memmove` with the counts right | 12 min, no lookups |
| 2 | `r02-memset-memcmp-reps` | clear an array, compare ranges, common prefix, all-one-byte | 10 min, no lookups |
| 3 | `r03-mem-from-scratch` | all five `mem` functions from a blank file | 15 min, no lookups |
