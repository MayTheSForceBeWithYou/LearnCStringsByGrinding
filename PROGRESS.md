# Progress

Your ledger. Tick an exercise only when it is clean everywhere: checks, forbidden-call check,
ASan/UBSan, and the fault sweep where tagged. For `[BUG]` exercises that includes a written
`ANSWER.md`. Tick a drill only after three reps on different days, each from a fresh copy
(`sh scripts/rep.sh <path>`). Use the Notes column for the thing you got wrong first; that is
the part worth rereading.

Legend: `[ ]` not started · `[~]` in progress · `[x]` clean everywhere

Folder names match `docs/SPEC.md` section 5. Units are added to this file as they are written.

## S00: Bytes, arrays and the terminator

`units/s00-bytes-and-nul` — read `LESSON.md` first.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-literal-sizes | `[PREDICT]` | |
| [ ] | e02-count-and-terminate | `[WRITE]` | |
| [ ] | e03-literal-write | `[BUG]` | |
| [ ] | e04-missing-nul | `[REPORT]` | |
| [ ] | e05-char-signedness | `[PREDICT]` | |
| [ ] | e06-decay-trace | `[TRACE]` | |
| [ ] | e07-string-lookup | `[LOOKUP]` | |

| | Drill | Rep 1 | Rep 2 | Rep 3 | Best time |
|---|---|---|---|---|---|
| [ ] | drills/r01-walk-reps | | | | |
| [ ] | drills/r02-init-reps | | | | |
| [ ] | drills/r03-escape-reps | | | | |

## S01: Length

`units/s01-length` — read `LESSON.md` first.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-my-strlen | `[WRITE]` | |
| [ ] | e02-fixed-fields | `[WRITE]` | |
| [ ] | e03-empty-wraps | `[BUG]` | |
| [ ] | e04-length-predict | `[PREDICT]` | |
| [ ] | e05-unterminated-report | `[REPORT]` | |
| [ ] | e06-narrow-length | `[BUG]` | |
| [ ] | e07-strlen-lookup | `[LOOKUP]` | |

| | Drill | Rep 1 | Rep 2 | Rep 3 | Best time |
|---|---|---|---|---|---|
| [ ] | drills/r01-strlen-reps | | | | |
| [ ] | drills/r02-end-pointer-reps | | | | |
| [ ] | drills/r03-count-reps | | | | |

## S02: The `mem` functions

`units/s02-mem-functions` — read `LESSON.md` first.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-my-memcpy-memmove | `[WRITE]` | |
| [ ] | e02-my-memset-memcmp-memchr | `[WRITE]` | |
| [ ] | e03-overlap-report | `[REPORT]` | |
| [ ] | e04-memset-args | `[BUG]` | |
| [ ] | e05-struct-compare | `[BUG]` | |
| [ ] | e06-mem-predict | `[PREDICT]` | |
| [ ] | e07-swap-reverse-rotate | `[WRITE]` | |
| [ ] | e08-mem-lookup | `[LOOKUP]` | |

| | Drill | Rep 1 | Rep 2 | Rep 3 | Best time |
|---|---|---|---|---|---|
| [ ] | drills/r01-gap-reps | | | | |
| [ ] | drills/r02-memset-memcmp-reps | | | | |
| [ ] | drills/r03-mem-from-scratch | | | | |

## S03: Copying

`units/s03-copying` — read `LESSON.md` first.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-my-strcpy-stpcpy | `[WRITE]` | |
| [ ] | e02-my-strncpy | `[WRITE]` | |
| [ ] | e03-my-strlcpy | `[WRITE]` | |
| [ ] | e04-strncpy-predict | `[PREDICT]` | |
| [ ] | e05-unterminated-name | `[BUG]` | |
| [ ] | e06-copy-policy | `[DESIGN]` | |
| [ ] | e07-struct-neighbour | `[BUG]` | |
| [ ] | e08-copy-lookup | `[LOOKUP]` | |

| | Drill | Rep 1 | Rep 2 | Rep 3 | Best time |
|---|---|---|---|---|---|
| [ ] | drills/r01-bounded-copy-reps | | | | |
| [ ] | drills/r02-fixed-field-reps | | | | |
| [ ] | drills/r03-stpcpy-chain-reps | | | | |

## S04: Concatenation

`units/s04-concatenation` — read `LESSON.md` first.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-my-strcat-strncat | `[WRITE]` | |
| [ ] | e02-my-strlcat | `[WRITE]` | |
| [ ] | e03-shlemiel | `[BUG]` | |
| [ ] | e04-strncat-size | `[BUG]` | |
| [ ] | e05-append-cursor | `[WRITE]` | |
| [ ] | e06-room-predict | `[PREDICT]` | |
| [ ] | e07-path-join | `[FAULT]` | |
| [ ] | e08-strcat-lookup | `[LOOKUP]` | |

| | Drill | Rep 1 | Rep 2 | Rep 3 | Best time |
|---|---|---|---|---|---|
| [ ] | drills/r01-append-reps | | | | |
| [ ] | drills/r02-room-math-reps | | | | |
| [ ] | drills/r03-join-reps | | | | |

## S05: Your first string-bug reports

`units/s05-first-reports` — read `LESSON.md` first. This unit has no drills.

| | Item | Tag | Notes |
|---|---|---|---|
| [ ] | e01-stack-overflow-report | `[REPORT]` | |
| [ ] | e02-global-overflow-report | `[REPORT]` | |
| [ ] | e03-overread-report | `[REPORT]` | |
| [ ] | e04-compiler-warnings | `[PREDICT]` | |
| [ ] | e05-silent-corruption | `[PREDICT]` `[NO-ASAN]` | |
| [ ] | e06-fortify-abort | `[REPORT]` `[NO-ASAN]` | |
