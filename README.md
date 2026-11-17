# Learn C Strings by Grinding

An extensive, grind-style course on strings in C, taught through the C standard library:
`<string.h>`, `<ctype.h>`, the `strtol` and `printf` families, line input, heap strings,
`argv` and the environment, locales and wide characters, UTF-8, and pattern matching. It
follows `LearnCMemoryByGrinding` and `LearnCDataStructuresByGrinding` and is built the same
way: lessons to read, exercises that do not link until you write the code, separate drills you
repeat until the skill is automatic, and a harness that checks everything under ASan, UBSan
and a leak checker.

What is different here: the curriculum is organised around the library's functions, every one
of which gets a reference card; your reimplementations are tested against the real libc on
thousands of generated inputs; and the reading and the exercise instructions are deliberately
descriptive.

> **Status:** Wave A complete. The spec, the harness and the first six units — **S00** to
> **S05** — are in place, each with its lesson, exercises, drills and solutions. The
> remaining fourteen units and the four capstones are specified in
> [`docs/SPEC.md`](docs/SPEC.md) and not yet written.

## Documents

- [`docs/SPEC.md`](docs/SPEC.md): the plan of record: curriculum, library coverage matrix, formats, harness.
- [`docs/AUTHORING.md`](docs/AUTHORING.md): content rules for authors.
- [`docs/AUTHORING-INFRA.md`](docs/AUTHORING-INFRA.md): build and harness mechanics.
- [`PROGRESS.md`](PROGRESS.md): your ledger of exercises and drills.

## Environment

- **Arch Linux** (native or WSL2; work under `~/dev/c`, not `/mnt/c`):
  `sudo pacman -S --needed base-devel clang perl man-pages`.
- **macOS on Apple Silicon**: `xcode-select --install`.

You need `make`, a C compiler, `nm`, `perl` and `man`. Later units need a UTF-8 locale
(`C.UTF-8` or `en_US.UTF-8`; `locale -a` lists what you have).

## How a unit works

1. Read `units/sNN-<slug>/LESSON.md`. It is the reading, not a summary of it: the mechanism,
   a reference card for each library function, worked examples, what the tools say, and
   questions to answer before you start.
2. Do the exercises in folder order. Each is a separate directory with a `README.md` that
   says what to build, how it must behave case by case, what the harness checks and how to
   read a failure.
3. Do every drill at least three times, on different days, from a fresh copy.

```bash
cd units/s03-copying/e01-my-strcpy-stpcpy
make check        # forbidden-call check, plain build, ASan/UBSan build; leak check in both
```

`[WRITE]`, `[DRILL]`, `[FAULT]` and `[DESIGN]` starters **do not build until you write the
code**: they declare your functions and the harness calls them, so the link fails with
`undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS). That
failure is your to-do list.

`[PREDICT]` and `[LOOKUP]` exercises have an `answers.txt`. You fill it in, and the program
grades it: each line is `ok`, `WRONG` or `blank`, and the right value is never printed.

Done means: `<id>: all checks passed` in every stage, silent sanitizers, no leak report.

```bash
sh scripts/rep.sh units/s03-copying/drills/r01-bounded-copy-reps    # a fresh rep of a drill
```

Reference solutions live in `solutions/`, for when you really need one. Each has a
`NOTES.md`: why it is correct, the edge cases, alternatives, the usual mistake, and how libc
does it.

## Byte notation

Strings are taught by looking at bytes. Everywhere in the course (lessons, READMEs,
`answers.txt`, failure messages) a buffer is written like this:

```
hello\0##        h e l l o, a terminator, and two bytes nobody has written
```

A character is itself, the terminator is `\0`, and `#` is the fill the harness puts in a
buffer before calling your code, so that a byte you did not write is visible.

## Course map

Folder names are the ones in `docs/SPEC.md` section 5.

| Unit | Folder | Topic | State |
|---|---|---|---|
| [S00](units/s00-bytes-and-nul/LESSON.md) | `units/s00-bytes-and-nul` | Bytes, arrays and the terminator | **complete** |
| [S01](units/s01-length/LESSON.md) | `units/s01-length` | Length: `strlen`, `strnlen`, `size_t` | **complete** |
| [S02](units/s02-mem-functions/LESSON.md) | `units/s02-mem-functions` | `memcpy`, `memmove`, `memset`, `memcmp`, `memchr` | **complete** |
| [S03](units/s03-copying/LESSON.md) | `units/s03-copying` | Copying: `strcpy`, `stpcpy`, `strncpy`, `stpncpy`, `snprintf`, `strlcpy` | **complete** |
| [S04](units/s04-concatenation/LESSON.md) | `units/s04-concatenation` | Concatenation: `strcat`, `strncat`, end pointers | **complete** |
| [S05](units/s05-first-reports/LESSON.md) | `units/s05-first-reports` | Your first string-bug reports | **complete** |
| S06 | `units/s06-comparison` | Comparison: `strcmp`, `strncmp`, `strcasecmp`, `qsort` | specified |
| S07 | `units/s07-ctype` | Character classes: `<ctype.h>` | specified |
| S08 | `units/s08-searching` | Searching: `strchr`, `strrchr`, `strstr`, `strpbrk` | specified |
| S09 | `units/s09-spans-tokens` | Spans and tokens: `strspn`, `strcspn`, `strtok`, `strtok_r` | specified |
| S10 | `units/s10-numbers-from-text` | Numbers from text: the `strtol` family | specified |
| S11 | `units/s11-formatted-output` | Formatted output: the `printf` family | specified |
| S12 | `units/s12-line-input` | Lines and formatted input: `fgets`, `getline`, `sscanf` | specified |
| S13 | `units/s13-heap-strings-streams` | Heap strings and memory streams | specified |
| S14 | `units/s14-system-strings` | `argv`, `getenv`, `strerror`, `strftime` | specified |
| S15 | `units/s15-locale-multibyte-wide` | Locales, multibyte and wide characters | specified |
| S16 | `units/s16-utf8-by-hand` | UTF-8 by hand | specified |
| S17 | `units/s17-pattern-matching` | `fnmatch` and POSIX regular expressions | specified |
| S18 | `units/s18-bug-zoo` | Bug hunt: the string bug zoo | specified |
| S19 | `units/s19-string-api-design` | Designing string APIs | specified |

Capstones (specified): `c1-mystring` (`<string.h>` from a blank file), `c2-minifmt`
(`vsnprintf`), `c3-conf` (an INI reader), `c4-csvstat` (a CSV reader and report).

## For authors

```bash
make check        # lint, harness self-test, verify every starter and solution
make verify CC=clang
sh scripts/verify-all.sh --only units/s03-copying
```
