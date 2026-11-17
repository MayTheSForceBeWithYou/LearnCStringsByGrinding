# Authoring infrastructure

How exercises plug into `common.mk`, `lib/` and `scripts/`. Content rules are in
`docs/AUTHORING.md`; the design rationale is in `docs/SPEC.md` section 8.

## Exercise Makefile

Every exercise, drill and solution directory has the same Makefile, with only values changed:

```make
TAG = WRITE
SRCS = main.c mystrncpy.c
EXPECT_SYMBOLS = my_strncpy my_stpncpy
FORBID = strncpy stpncpy strcpy stpcpy strlen strnlen memcpy memmove memset
STUDENT_SRCS = mystrncpy.c
include ../../../common.mk
```

| Directory | Include line |
|---|---|
| `units/sNN/eNN-*/`, `solutions/sNN/eNN-*/` | `include ../../../common.mk` |
| `units/sNN/drills/rNN-*/`, `solutions/sNN/drills/rNN-*/` | `include ../../../../common.mk` |
| `capstones/cN-*/` | `include ../../common.mk` |
| `solutions/capstones/cN-*/` | `include ../../../common.mk` |

- `SRCS` defaults to `main.c`; list every `.c` of a multi-file exercise. Local `.h` files are
  dependencies automatically.
- Optional: `ARGS`, `EXTRA_CFLAGS`, `LDLIBS`, `ASAN_EXTRA`, `TIMEOUT` (seconds per program run,
  default 20). One line per variable: the harness reads them with `sed`.
- `FORBID`: libc functions the student's code must not call (reimplementation exercises).
  `STUDENT_SRCS` names the files it applies to and is required with it. Those files are
  compiled with `-fno-builtin`; `make check` runs `scripts/forbid.sh` on their plain-build
  objects first (`== forbid`). Keep the harness in `main.c` and the student's code in its own
  file, because the harness calls the very functions that are forbidden (as the oracle).
- `WARN_SRC`: lesson demos only: a source compiled (never linked) by `make capture TOOL=warn`.
- `TAG` words: `WRITE DRILL BUG REPORT PREDICT TRACE FAULT DESIGN`, plus `NO-ASAN` to skip the
  sanitizer stage. `EXPECT_SYMBOLS` is required for WRITE, DRILL, FAULT and DESIGN.

## Targets (inside an exercise)

| Command | Does |
|---|---|
| `make` / `make run` | build `build/plain/prog` and run it (dsalloc leak check on) |
| `make asan` | build `build/asan/prog` with ASan + UBSan and run it |
| `make fault` | run the `FAIL_AT=1..N` sweep on the ASan build |
| `make check` | all stages that apply; prints `== forbid`, `== plain+dsalloc`, `== asan+ubsan+dsalloc[+lsan]`, `== fault-sweep`, `== check: OK` |
| `make forbid` | only the forbidden-call check |
| `make clean` | remove `build/` |
| `make valgrind` | Linux only, optional: memcheck on the plain build |
| `make capture TOOL=asan` | write `capture.txt` with a source hash header (`TOOL=run`, `asan` or `warn`; `CAPTURE=name.txt`; `ARGS=...`) |
| `make build-objs` | compile only (the harness uses it to tell compile errors from link errors) |
| `make B=/tmp/x ...` | build somewhere else (the harness always does) |
| `make WERROR=1 ...` | add `-Werror` (the harness always does) |

## Flags

`-std=c17 -Wall -Wextra -Wpedantic -g -U_FORTIFY_SOURCE -I lib -include lib/dsalloc.h`, no
optimisation; the sanitizer build adds
`-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer` to compile and
link. `ASAN_OPTIONS` is `detect_leaks=1` on Linux and `detect_leaks=0` on macOS, always with
`abort_on_error=1:halt_on_error=1:allocator_may_return_null=1`; `UBSAN_OPTIONS` is
`print_stacktrace=1:halt_on_error=1`.

Because `dsalloc.h` defines `_POSIX_C_SOURCE 200809L` before any system header, `strdup`,
`strndup`, `strnlen`, `stpcpy`, `stpncpy`, `strtok_r`, `getline`, `fmemopen` and
`open_memstream` are declared everywhere. Do not define other
feature-test macros. A source may repeat `#define _POSIX_C_SOURCE 200809L` (identical
redefinition is allowed); any other value is a lint error.

## lib/

- **`course.h`**: `check(cond, msg)`, `checks_done(id)`, `EXIT_OOM` (3), `EXIT_LEAK` (4),
  `EXIT_NO_LOCALE` (5).
- **`bytes.h`**: `bytes_notation()` and `bytes_show()`: a buffer in byte notation
  (`hello\0##`). Everything that prints a buffer uses it.
- **`strcheck.h`**: `check_str(got, want, msg)`, `check_bytes(got, want, n, msg)`,
  `check_guard(p, msg)`, `sign_of(v)`. On failure they print both sides in byte notation.
  Static inline, because `course.h` counts failures per translation unit: call them from the
  file that calls `checks_done()`.
- **`guard.h`**: `guard_new(cap)` gives a destination of exactly `cap` bytes filled with `#`
  (`cap` may be 0); `guard_copy(src, n)` a source of exactly `n` bytes; `guard_ok`,
  `guard_reset`, `guard_free`. Plain build: 0xA5 fences checked by `guard_ok`. ASan build: an
  exact-size heap block, so one byte too many is a `heap-buffer-overflow` "0 bytes after
  N-byte region" (a zero-capacity buffer reports `use-after-poison`). Use it for every
  function that writes into a caller's buffer, and `guard_copy` for every source whose end
  matters. At most 64 live at once. Not counted by dsalloc.
- **`oracle.h`**: `orc_seed`, `orc_next`, `orc_below`, `orc_string(buf, maxlen, alphabet)`,
  `orc_case()`: the same generated strings on every platform, for comparing a
  reimplementation with libc. Print the seed and `orc_case()` on a mismatch, show both
  results in byte notation, and stop comparing that function after the first mismatch.
- **`answers.h`**: the `answers.txt` grader: `answers_load`, `answer_bytes`, `answer_int`,
  `answer_str`, `answer_hash`, `answers_done`. Compute the truth by running the code. For
  facts that cannot be computed, `answer_hash(key, h)` compares a hash of `"key=answer"`;
  get `h` by running the solution with `ANSWERS_SHOW_HASH=1`. The starter ships `answers.txt`
  with every value blank; the solution ships it filled in.
- **`utf8locale.h`**: `course_utf8_locale()` selects `C.UTF-8`, `en_US.UTF-8` or `UTF-8`
  without reading the environment, or exits 5 with instructions. (Not named `locale.h`:
  `lib/` is on the include path and would hide the standard header.)
- **`dsalloc.h` / `dsalloc.c`**: allocation accounting and fault injection, in every build.
  - Student code calls plain `malloc`, `calloc`, `realloc`, `strdup`, `strndup`, `free`.
  - `FAIL_AT=N ./prog`: the Nth allocation call returns NULL (`errno = ENOMEM`). A failed
    `realloc` leaves the old block alive.
  - `FA_COUNT=1 ./prog`: prints `dsalloc: N allocation calls` at exit.
  - At exit, live blocks are reported with their allocation sites and the exit status becomes 4.
    `DSALLOC_LEAKS=0` turns that off (lesson demos only, never in an exercise Makefile).
  - Harness-only queries: `ds_live_blocks()`, `ds_live_bytes()`, `ds_alloc_calls()`,
    `ds_fail_nth(n)` (the nth allocation call from now fails; 0 cancels).
  - Limits: do not take the address of the allocation functions (except `free`), and do not
    expect allocations made inside libc (`getline`, `fopen`) to be counted or failed. Keep
    exercises that depend on them out of `[FAULT]`. On Linux, LSan still checks them.

## scripts/

| Script | Use |
|---|---|
| `verify-all.sh [--only PATH]... [ROOT]` | the harness; one row per item; exit non-zero on any failure |
| `selftest.sh` | tests of the harness on the current machine and `CC` |
| `lint.sh [ROOT]` | portability and structure checks |
| `fault-sweep.sh BIN [ARGS]` | called by `make fault` / `make check` |
| `run-timeout.sh SECS CMD...` | portable time limit; 124 on timeout, 128+N on signal |
| `undef-syms.sh LOG` | undefined symbols from a GNU ld, lld or Apple ld log |
| `srchash.sh FILE...` | portable sha256 |
| `rep.sh PATH` | fresh copy of a drill or exercise into `reps/` |
| `forbid.sh "NAMES" OBJ...` | fail if an object references a forbidden function (`nm -u`) |

`--only PATH` is a prefix match relative to the root; the mirrored solution comes along:
`sh scripts/verify-all.sh --only units/s03-copying`.

## Reading verify-all output

```
ITEM                                        TAG      KIND     RES   DETAIL
units/s03-copying/e01-my-strcpy-stpcpy      WRITE    starter  PASS  scaffold OK: link fails on own symbol(s): my_stpcpy my_strcpy
solutions/s03-copying/e01-my-strcpy-stpcpy  WRITE    solution PASS  check ok: forbid, plain+dsalloc, asan+ubsan+dsalloc+lsan
units/s03-copying/e04-strncpy-predict       PREDICT  starter  PASS  builds; blank answers.txt fails the grader
units/s03-copying/e05-unterminated-name     BUG      starter  PASS  builds; check fails at: plain+dsalloc
```

A failing row is followed by the last lines of its log. `SKIP` rows name anything the harness
found but does not verify. The last two lines count scaffolds, solutions, failures and skips.

## Captures

Tool output pasted anywhere must come from the exact shipped source:

```
make capture TOOL=asan                                   # capture.txt
make capture TOOL=run
make capture TOOL=asan ARGS=overread CAPTURE=capture-asan.txt
make capture TOOL=warn CAPTURE=capture-warn.txt          # compiler diagnostics for $(WARN_SRC), at -O2
```

Every capture starts with `# source-sha256: <hash>` over `SRCS`, local headers and
`WARN_SRC`; verify-all recomputes it for every `capture*.txt` (REPORT and BUG starters,
solutions, lesson demos) and flags a mismatch as STALE. Captures are made on Arch Linux with
gcc; say so where clang or macOS output differs. **Any edit to a captured source means
re-capturing every capture in that directory and updating every place that quotes one**:
addresses, PIDs and line numbers all change.

A `[BUG]`, `[REPORT]` or `[PREDICT]` source must compile without warnings on gcc and clang
although it contains a defect. gcc warns about `strncpy` and `snprintf` calls whose sizes it
can see, even at `-O0` (`-Wstringop-truncation`, `-Wformat-truncation`). Put such calls
behind a function that takes the size as a parameter, or drive them from a table; never
silence a warning with a pragma.

## Lesson demos

`units/sNN-*/lesson-demo/` holds programs whose output a lesson quotes. Same Makefile shape
(`TAG = PREDICT`); verify-all builds them and checks their `capture*.txt` hashes.

## Writing an exercise: the order that works

1. Header with the prototypes (one line each) and full contract comments; student file with
   only includes and the marker line; `main.c` harness; Makefile.
2. `sh scripts/verify-all.sh --only <path>`: the starter must be "scaffold OK".
3. The solution, in `solutions/<mirror>/`: copy every starter file, then write below the
   marker. Verify again with `CC=cc` and `CC=clang`.
4. A deliberately wrong implementation in a scratch directory outside the tree, to see that
   the harness rejects the usual mistake and to get a real FAIL line for the README.
5. The README (sections of `docs/SPEC.md` 8.4), then `NOTES.md`.
6. `make check` at the root.
