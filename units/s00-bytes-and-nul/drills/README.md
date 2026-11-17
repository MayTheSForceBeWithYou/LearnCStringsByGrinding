# S00 drills

Drills are short, separate repetitions of one skill. They do not build on each other, and they
never touch the exercises. Each is graded by the same harness as the exercises: the starter
fails to link until you write the functions, and `make check` must pass with ASan, UBSan and
the leak check silent.

Do every drill at least three times, on different days, from a fresh copy each time:

```bash
sh scripts/rep.sh units/s00-bytes-and-nul/drills/r01-walk-reps
cd reps/s00-bytes-and-nul/r01-walk-reps/01 && make check
```

`reps/` is ignored by git. Each rep gets its own numbered folder, so you can compare today's
attempt with last week's. Time yourself; the target is in each drill's README.

These three are the course's basic motions. r01 reads a string to its terminator; r02 writes
into a buffer without passing its capacity; r03 shows you what is really in the bytes when
neither of the first two did what you expected. Every unit from here on is one of those three
with a library function in the middle, so the time spent making them automatic is bought back
twenty times.

| # | Folder | Skill | Target |
|---|---|---|---|
| 1 | `r01-walk-reps` | five questions about a string, each answered by one pointer walk | 8 min, no lookups |
| 2 | `r02-init-reps` | fill a buffer by hand: empty, repeated, copied, always terminated | 10 min, no lookups |
| 3 | `r03-escape-reps` | render a buffer so that `\0`, `\n` and `\xHH` are visible | 15 min, no lookups |
