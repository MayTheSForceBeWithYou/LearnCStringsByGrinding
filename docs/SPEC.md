# Spec: Learn C Strings by Grinding

Version 0.2 (Phase 1 built: harness and sample unit S03). Owner: Bruce (harness, CI, technical review). This file is the
plan of record. When it disagrees with another document, this file wins; fix the other document
and tell its owner.

## 1. Why this exists

`LearnCMemoryByGrinding` spent one unit on strings (M07: `strdup`, exact sizing, `snprintf` to
measure). `LearnCDataStructuresByGrinding` spent one more (D20: string builders, gap buffers,
ropes). Both treated strings as a vehicle for something else. Neither taught the C standard
library's string functions, and that is where C programs actually go wrong: `strncpy` that does
not terminate, `strncat` given the wrong size, `strtok` that edits the caller's data, `atoi`
that cannot report an error, `isalpha` called with a negative `char`, `%s` with no width.

This course is the third in the series: an extensive, grind-style course on **strings in C,
taught through the standard library**. It is built the same way as its two predecessors: same
repository shape, same tags, same "starter does not link until you write the code" rule, same
separate `solutions/` tree, same portable harness, drills carried over from the data structures
course. It differs in four ways:

- **Library first.** The curriculum is organised around the functions of `<string.h>`,
  `<ctype.h>`, `<stdlib.h>`, `<stdio.h>`, `<wchar.h>` and their POSIX neighbours. Every function
  gets a reference card in a lesson, is used in an exercise, and (where it is small enough) is
  reimplemented by the student and tested against the real one. Section 6 is the coverage
  matrix; a function missing from it is a gap in the course.
- **Descriptive reading.** Lessons are longer than in the earlier courses (250-550 lines) and
  exercise READMEs explain the background, the exact behaviour, the edge cases and what the
  harness checks. Nathan's request, verbatim: "Be descriptive in exercise instructions and
  lesson reading material." Section 8 defines what that means and where it stops.
- **Oracle testing.** A student's `my_strstr` is checked against libc's `strstr` on thousands
  of generated inputs, not against a handful of hand-written cases.
- **Graded predictions.** `[PREDICT]` and `[LOOKUP]` answers go in `answers.txt` and the
  program grades them, so a prediction is either right or visibly wrong.

## 2. Goals

After the course, without looking anything up beyond `man`, the student can:

- state, for every function in section 6, what it requires of its arguments, what it writes,
  what it returns, and what makes the call undefined;
- say for any buffer and any call whether the result is NUL-terminated, and how truncation is
  detected;
- choose the right function for a job (copy, append, compare, search, split, convert, format,
  read a line) and name the specific defect of each wrong choice;
- write every function of C17 `<string.h>` from a blank file, correct for empty strings, bytes
  above 0x7F, overlapping buffers and `n == 0`;
- convert text to numbers with full error detection (`strtol` family, `errno`, `endptr`), and
  numbers to text with exact sizing (`snprintf`);
- read input of any length without overflow (`fgets`, `getline`, bounded `sscanf`);
- handle bytes versus characters: UTF-8 by hand, and multibyte/wide conversion through
  `<wchar.h>` under a locale;
- read an ASan, UBSan, compiler or glibc report about a string bug and name the line and the
  mechanism;
- design a string API whose contract states ownership, capacity, termination and truncation.

### Anti-goals

- Not a text-algorithms course. No KMP, Boyer-Moore, suffix arrays, edit distance or regex
  engines. Naive search is implemented once; the rest is library use.
- Not a Unicode course. UTF-8 encoding, validation and boundaries only. No normalization,
  grapheme clusters, display width or case folding beyond what `towupper` does.
- Not a repeat of M07 or D20. Heap sizing and string builders are assumed; S13 and S19 extend
  them in the direction of the library (`open_memstream`, views).
- No external libraries. C17 standard library plus POSIX.1-2008 only.
- No threads. Thread-safety of `strtok` and `strerror` is explained, not exercised.
- No C23-only features. Lessons say what C23 changes (`strdup` standardised, `memccpy`,
  `memset_explicit`, const-preserving `strchr`); exercises compile as C17.

## 3. Environment

Identical to the data structures course.

| | Arch Linux (WSL2 or native) | macOS, Apple Silicon |
|---|---|---|
| Compilers | `gcc`, `clang` (pacman) | Apple clang (Xcode Command Line Tools) |
| Linker | GNU ld (bfd) by default; lld if chosen | Apple ld |
| make | GNU make 4.x | GNU make 3.81 (`/usr/bin/make`) |
| `/bin/sh` | bash in sh mode | bash 3.2 in sh mode (CI also runs dash on Ubuntu) |
| Sanitizers | ASan, UBSan, LSan | ASan, UBSan (no LSan on arm64) |
| libc | glibc | Apple libc (BSD-derived) |
| UTF-8 locale | `C.UTF-8` (built into glibc) | `en_US.UTF-8` |
| `man` | man-pages (section 3, section 7) | BSD man pages (section 3, section 5/7) |

Work under `~/dev/c` on WSL, never under `/mnt/c`. The student needs `make`, a C compiler,
`perl`, `nm` and `man`. Nothing else.

Two libcs is a feature here, not a cost. Where glibc and Apple libc legitimately differ
(`strerror` text, `mbrtowc` in the "C" locale, collation order, `%s` with NULL), the lesson says
so and the harness checks only what the standard guarantees.

## 4. Structure

### 4.1 Units, lessons, exercises, drills

The course has **20 units** (`S00`-`S19`) and **4 capstones** (`C1`-`C4`), **164 exercises** and
**51 drills** (section 5). Unit `S07` is the folder `units/s07-<slug>/`. (The unit letter `S`
and the size `S` never collide in practice: a unit id always carries two digits.)

```
units/s03-copying/
├── LESSON.md                  # the reading (section 8.1)
├── e01-my-strcpy-stpcpy/      # exercise: README.md, Makefile, starter sources
├── e02-my-strncpy/
├── ...
├── lesson-demo/               # programs whose output LESSON.md quotes (capture*.txt)
└── drills/
    ├── README.md              # the drill table and the rep workflow
    └── r01-bounded-copy-reps/ # drill: README.md, Makefile, starter sources
solutions/s03-copying/
├── e01-my-strcpy-stpcpy/      # complete program + NOTES.md
├── ...
└── drills/r01-bounded-copy-reps/
```

- **Exercise id**: `sNN-eNN-<slug>` (e.g. `s03-e02-my-strncpy`), always the full form.
- **Drill id**: `sNN-rNN-<slug>`. **Capstone id**: `cN-<name>`.
- The harness prints `<id>: all checks passed` on success.

### 4.2 Exercises versus drills

Unchanged from the data structures course (its section 4.2): an exercise is done once, to
"clean everywhere"; a drill is one skill, 3-8 small functions, 10-25 minutes, done at least
three times on different days, each rep from a fresh copy made by `scripts/rep.sh`.

### 4.3 Tags

| Tag | Student... | Starter state |
|---|---|---|
| `[WRITE]` | implements functions until the checks pass | does **not** link |
| `[DRILL]` | implements a set of small functions, repeatedly | does **not** link |
| `[FAULT]` | survives every forced allocation failure (`FAIL_AT=1..N`) | does **not** link |
| `[DESIGN]` | writes the contract in the header, then implements it | does **not** link |
| `[BUG]` | finds and fixes the advertised defect, explains it in `ANSWER.md` | builds, then fails a check or a sanitizer |
| `[REPORT]` | pinpoints the bug from a real captured report before running anything | builds; `capture.txt` matches source |
| `[PREDICT]` | fills `answers.txt` with the outcome, then runs the grader | builds, then fails `make check` |
| `[TRACE]` | draws buffer contents and pointer positions at marked points | builds |
| `[LOOKUP]` | answers from `man` only: values in `answers.txt`, quotes in `ANSWER.md` | builds, then fails `make check` |
| `[NO-ASAN]` | (marker) a demonstration that deliberately runs without sanitizers | |

`[LOOKUP]` is a first-class tag here (it was a README-only label before), because reading the
manual page of a library function is a skill this course trains in every unit. Its Makefile
uses `TAG = PREDICT`. `[EXPLAIN]` and `[BREAK]` may appear in README titles; they also build
as `TAG = PREDICT`.

**Composition per unit:** at most 5 `[WRITE]`; at least 1 `[BUG]` or `[REPORT]`; at least 1
`[PREDICT]` or `[TRACE]`; exactly 1 `[LOOKUP]`; at least 1 `[FAULT]` in every unit whose
exercises allocate (S04, S08-S13, S15, S17, S19); at least 2 drills. Exceptions: S05 (reports
only: no `[WRITE]`, no drills, no `[LOOKUP]`) and S18 (the bug zoo: `[BUG]` only, no drills).

**Sizes:** XS (< 15 lines of student code, drills only), S (< 40), M (40-150), L (150-400,
capstones only).

### 4.4 The rule that never bends

**Every `[WRITE]`, `[DRILL]`, `[FAULT]` and `[DESIGN]` starter fails to link until the student
writes the real code, and it fails on exactly its own symbols.** No stub bodies, no `return 0;`
placeholders, no `#error`, no stop lines, no `TODO` that compiles. `EXPECT_SYMBOLS` lists the
functions; the harness requires the linker's undefined-symbol set to equal that list. This is
section 4.4 of the data structures spec, unchanged.

### 4.5 Naming: `my_` and the libc names

- A student reimplementation of a libc function is named `my_<name>` and has the libc
  prototype exactly (`size_t my_strlen(const char *s);`). No course file ever defines a symbol
  that libc defines; `scripts/lint.sh` rejects it. (Defining `strlen` yourself is undefined
  behaviour and, with ASan's interceptors, a confusing afternoon.)
- Functions that are *not* in the portable set (section 10) but are worth knowing are written
  by the student under `my_` as well: `my_strlcpy`, `my_strlcat`, `my_strsep`, `my_memmem`,
  `my_strcasestr`, `my_memccpy`.
- Everything else gets a short module prefix chosen in the starter header (`cur_`, `sv_`,
  `tok_`, `utf8_`).

### 4.6 What "do not call the library" means

Most exercises *require* the library. A reimplementation exercise forbids the function being
reimplemented and its close relatives. The Makefile says so:

```make
FORBID = strlen strnlen
```

The harness runs `nm -u` on the student's object file and fails if any listed symbol is
referenced (section 9.3). The README states the forbidden list and the reason in one sentence.
Nothing is forbidden silently, and nothing is forbidden without a reason.

## 5. Curriculum

Each unit lists its goal, the library functions it covers, prerequisites, exercises and
drills. Folder names are frozen by these tables. To rename anything, change this file first.

How the earlier courses feed in: M02-M04 (allocation, sizing, `realloc`) → S13, every
`[FAULT]`; M05 (ownership) → S13, S19; M06 (partial failure) → every `[FAULT]`; M07 (heap
strings) → S04, S11, S13; M02b, M11 (reading reports) → S05, every `[BUG]` and `[REPORT]`;
D00 (cost model) → S01, S04; D01 (dynamic arrays) → S09, S12; D20 (builders) → S13, S19;
D21 (`qsort` comparators) → S06. The memory course is a hard prerequisite. The data structures
course is recommended, not required: a student who has only done the memory course can take
this course and skip nothing.

### S00: Bytes, arrays and the terminator

Goal: a C string is a convention, not a type. Arrays versus pointers to `char`, string
literals (type, storage, immutability, concatenation, `char a[3] = "abc"`), length versus size,
`sizeof` versus `strlen`, `char` signedness, escape sequences, what "terminated" means and
what every library function assumes about it.
Library: `strlen` (as a black box), `sizeof`, `printf("%s")`, `puts`.
Prerequisites: memory M01-M02. 7 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-literal-sizes` | PREDICT | | `sizeof` and `strlen` for fourteen declarations: `char a[] = "abc"`, `char *p = "abc"`, `char b[3] = "abc"`, `"ab" "cd"`, `"a\0b"`, `""`, `char m[3][8]`, a `char *v[]` table |
| `e02-count-and-terminate` | WRITE | S | no library at all: `str_len` by index, `str_end` by pointer, `buf_is_terminated(buf, cap)`, `buf_set(buf, cap, c, n)` that always terminates |
| `e03-literal-write` | BUG | | a function upper-cases its argument in place and is called with a string literal; the program dies with SIGSEGV. Fix the caller, not the callee, and say why |
| `e04-missing-nul` | REPORT | | ASan `stack-buffer-overflow` READ inside `strlen`, caused by `char id[4] = "A123"`: find the declaration from the report alone |
| `e05-char-signedness` | PREDICT | | comparisons, promotions and array indexes for the byte 0xE9 held in `signed char`, `unsigned char` and `int`; which expressions differ and why plain `char` is not portable |
| `e06-decay-trace` | TRACE | | draw where the bytes live for `char a[]`, `char *p`, `char *v[3]`, `char m[3][8]`, and what each `sizeof` and each `&x[0]` is |
| `e07-string-lookup` | LOOKUP | | from `man 3 string` and `man 3 strlen`: the header, the return type, which parameters are `const`, and the sentence that says what happens without a terminator |
| `drills/r01-walk-reps` | DRILL | XS | length, count of a character, last character, is-empty, pointer to the terminator: each by pointer walk |
| `drills/r02-init-reps` | DRILL | XS | fill a buffer by hand: empty string, n copies of a character, a literal byte by byte, always terminated, never past `cap` |
| `drills/r03-escape-reps` | DRILL | XS | `byte_escape`: render a buffer with `\0`, `\n`, `\t` and `\xHH` visible (the debugging tool you will want all course) |

### S01: Length

Goal: what `strlen` costs and what it trusts; bounded measurement; `size_t` arithmetic and its
wrap at zero; lengths that do not fit their type.
Library: `strlen`, `strnlen` (POSIX), `size_t`, `ptrdiff_t`, `SIZE_MAX`.
Prerequisites: S00; data structures D00 helps. 7 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strlen` | WRITE | S | `my_strlen` and `my_strnlen`, oracle-tested against libc on generated strings, including empty and `maxlen == 0`. `FORBID = strlen strnlen` |
| `e02-fixed-fields` | WRITE | S | fixed-width record fields that are NUL-padded but not necessarily terminated (a 16-byte name field holding 16 characters): `field_len`, `field_eq`, `field_to_cstr(dst, cap, field, width)` using `strnlen` and `memcpy` |
| `e03-empty-wraps` | BUG | | `for (i = 0; i <= strlen(s) - 1; i++)` on the empty string: `size_t` wraps to `SIZE_MAX`, ASan reports the over-read |
| `e04-length-predict` | PREDICT | | `strlen`, `strnlen` and `sizeof` for sixteen expressions: embedded NUL, pointer plus offset, `strnlen(s, 0)`, a field with no terminator |
| `e05-unterminated-report` | REPORT | | ASan `heap-buffer-overflow` READ with `strlen` as the top frame: use the "allocated by" stack and the region size to find the copy that dropped the terminator |
| `e06-narrow-length` | BUG | | a record stores its name length in a `uint8_t`; a 300-character name wraps to 44 and the copy is silently short. A check fails; no tool reports anything |
| `e07-strlen-lookup` | LOOKUP | | `man 3 strlen`, `man 3 strnlen`: which standard each conforms to, and the feature-test macro `strnlen` needs |
| `drills/r01-strlen-reps` | DRILL | XS | `strlen` three ways (index, pointer difference, bounded), each from memory |
| `drills/r02-end-pointer-reps` | DRILL | XS | pointer to the terminator, pointer to the last character, length from two pointers, remaining room from `end - p` |
| `drills/r03-count-reps` | DRILL | XS | count occurrences of a character, words, lines, and the longest run of one character |

### S02: The `mem` functions

Goal: the byte-level functions that every `str` function is built on. Lengths are explicit,
nothing stops at NUL. Overlap, `restrict`, why `memmove` exists, `memcmp` on structs with
padding, `memset`'s argument order and its `int` value converted to `unsigned char`.
Library: `memcpy`, `memmove`, `memset`, `memcmp`, `memchr`.
Prerequisites: S01. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-memcpy-memmove` | WRITE | M | `my_memcpy` and `my_memmove` (forward copy when `dst < src`, backward otherwise), oracle-tested over every overlap offset from -32 to +32. `FORBID = memcpy memmove` |
| `e02-my-memset-memcmp-memchr` | WRITE | S | the other three, with `unsigned char` comparison and `n == 0`. `FORBID = memset memcmp memchr` |
| `e03-overlap-report` | REPORT | | ASan `memcpy-param-overlap`: read the two address ranges in the first line and say by how many bytes they overlap |
| `e04-memset-args` | BUG | | `memset(buf, n, 0)`: value and count swapped, so nothing is cleared and stale bytes leak into the next record |
| `e05-struct-compare` | BUG | | `memcmp` on two structs whose fields are equal but whose padding bytes differ; the equality test fails. Fix by comparing fields |
| `e06-mem-predict` | PREDICT | | sign of `memcmp` for eight pairs; `memchr` with `n == 0`; buffer contents after overlapping `memmove`s; the byte stored by `memset(p, 0x141, 4)` |
| `e07-swap-reverse-rotate` | WRITE | S | `mem_swap`, `mem_reverse` and `mem_rotate_left` for arbitrary element sizes, built from `memcpy`/`memmove` with a bounded stack scratch buffer |
| `e08-mem-lookup` | LOOKUP | | `man 3 memcpy`, `man 3 memmove`: the `restrict` qualifiers in the prototypes and the sentence about overlapping objects |
| `drills/r01-gap-reps` | DRILL | XS | open a gap, close a gap, delete a range, insert bytes: `memmove` with the lengths right every time |
| `drills/r02-memset-memcmp-reps` | DRILL | XS | zero an array of structs with `sizeof *p`, equal ranges, common-prefix length, "all bytes equal c" |
| `drills/r03-mem-from-scratch` | DRILL | S | all five functions from a blank file in 15 minutes |

### S03: Copying

Goal: every way to copy a string into a buffer, and what each does when the source does not
fit. `strcpy` (unbounded), `strncpy` (pads; does not terminate on truncation; designed for
fixed-width fields), `stpcpy` (returns the end), `snprintf(dst, cap, "%s", src)` (terminates;
returns the needed length), and the BSD `strlcpy` contract, written by hand because it is not
in the portable set.
Library: `strcpy`, `strncpy`, `stpcpy`, `stpncpy` (POSIX), `snprintf`.
Prerequisites: S02. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strcpy-stpcpy` | WRITE | S | `my_strcpy` (returns `dst`) and `my_stpcpy` (returns a pointer to the terminator it wrote). `FORBID` lists the copy functions, `strlen`, `memcpy` and `memmove` |
| `e02-my-strncpy` | WRITE | S | `my_strncpy` and `my_stpncpy` with the exact libc semantics: exactly `n` bytes written, NUL padding when the source is short, no terminator when it is not. The harness compares all `n` bytes and the guard bytes beyond them |
| `e03-my-strlcpy` | WRITE | S | `my_strlcpy(dst, src, size)`: always terminates when `size > 0`, returns `strlen(src)`, so `ret >= size` means truncated. Reads all of `src` exactly once. The oracle is `snprintf(dst, size, "%s", src)` |
| `e04-strncpy-predict` | PREDICT | | all bytes of an 8-byte buffer (pre-filled with `'#'`) after each of ten `strncpy`, `stpncpy` and `snprintf` calls, plus each return value |
| `e05-unterminated-name` | BUG | | `strncpy(p->name, src, sizeof p->name)` with a source of exactly that length; a later `strlen` runs off the field. ASan reports on the heap; fix the copy, not the reader |
| `e06-copy-policy` | DESIGN | M | three bounded copies with three policies: `copy_trunc` (truncate, report), `copy_strict` (fail, leave `dst` as the empty string), `copy_n` (source length known). Write the contract for each in the header first |
| `e07-struct-neighbour` | BUG | | `strcpy` into a 16-byte field overflows into the next field of the same struct. ASan is silent (the write stays inside one object); only the corrupted neighbour fails a check. Explain why the tool missed it |
| `e08-copy-lookup` | LOOKUP | | `man 3 strcpy`, `stpcpy`, `strncpy`, `stpncpy`: nine facts (return values, padding, termination, header, overlap, standard), each with the supporting sentence quoted |
| `drills/r01-bounded-copy-reps` | DRILL | XS | copy with a capacity four ways: `strlcpy`-style loop, `snprintf`, `strlen` plus `memcpy` plus terminator, strict (fail if it does not fit) |
| `drills/r02-fixed-field-reps` | DRILL | XS | write a string into a NUL-padded field and read it back out terminated |
| `drills/r03-stpcpy-chain-reps` | DRILL | XS | build strings from parts with an end pointer, never rescanning: two parts, `a/b/c`, n repeats, a bounded list |

### S04: Concatenation

Goal: appending without rescanning and without overflow. Why `strcat` in a loop is quadratic;
what the `n` of `strncat` really bounds (characters appended, not the destination size); the
`strlcat` contract; the end-pointer and remaining-room idiom; appending formatted text.
Library: `strcat`, `strncat`, `snprintf`, `vsnprintf`.
Prerequisites: S03; memory M07. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strcat-strncat` | WRITE | S | `my_strcat` and `my_strncat` (appends at most `n` characters, then always a terminator: up to `n + 1` bytes written). `FORBID = strcat strncat` |
| `e02-my-strlcat` | WRITE | S | `my_strlcat(dst, src, size)`: returns the length it tried to create; correct when `dst` has no terminator within `size` |
| `e03-shlemiel` | BUG | | 20,000 words joined with `strcat` in a loop; correct output, fails its CPU budget. Fix with an end pointer; keep the function's signature |
| `e04-strncat-size` | BUG | | `strncat(dst, src, cap)` where `cap` is the destination's total size: ASan `stack-buffer-overflow`. Write the correct third argument and explain the formula |
| `e05-append-cursor` | WRITE | M | an allocation-free bounded appender: `cur_init(buf, cap)`, `cur_puts`, `cur_putc`, `cur_printf` (via `vsnprintf`), `cur_truncated`. Truncation is sticky; the buffer is terminated after every call |
| `e06-room-predict` | PREDICT | | for twelve append calls: the bytes written, the return value, and the remaining room afterwards |
| `e07-path-join` | FAULT | M | `path_join(dir, name)` and `path_join_n(parts, n)` returning heap strings: exactly one separator between parts whatever the inputs end and start with; all-or-nothing under every `FAIL_AT` |
| `e08-strcat-lookup` | LOOKUP | | `man 3 strcat`: the sentence that says how many bytes `strncat` may write, and the worked size formula in the page |
| `drills/r01-append-reps` | DRILL | XS | append a string, a character, and a decimal number at an end pointer with a remaining-room check |
| `drills/r02-room-math-reps` | DRILL | XS | `room_left`, `will_fit`, `size_needed` for two- and three-part concatenations, with `size_t` overflow checks |
| `drills/r03-join-reps` | DRILL | S | join n strings with a separator into a fixed buffer, reporting truncation |

### S05: Your first string-bug reports

Goal: read what the tools say about string bugs, early, because every later unit produces
these reports. Placed here for the same reason the memory course has M02b. ASan's first line
(`stack-buffer-overflow`, `global-buffer-overflow`, `heap-buffer-overflow`; READ or WRITE; the
size); an interceptor frame (`strcpy`, `strlen`, `printf`) on top of the stack and your frame
under it; the "is located N bytes after" line; what ASan cannot see (overflow inside one
struct); compiler warnings (`-Wformat`, `-Wstringop-overflow`, `-Wstringop-truncation`); glibc's
`*** buffer overflow detected ***` under `_FORTIFY_SOURCE`.
Library: none new. Prerequisites: S04; memory M02b. 6 exercises, no drills, no `[WRITE]`.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-stack-overflow-report` | REPORT | | `stack-buffer-overflow` WRITE of size 13 with `strcpy` on top: name the buffer, its size and the overflowing line from the frame list and the stack-object table |
| `e02-global-overflow-report` | REPORT | | `global-buffer-overflow`: read the "is located 0 bytes after global variable" line and the variable's defining file and line |
| `e03-overread-report` | REPORT | | a READ inside `printf`'s `%s`: distinguish "wrote too much" from "read without a terminator" by the access kind and the frame |
| `e04-compiler-warnings` | PREDICT | | ten snippets (shipped as `snippets/*.c.txt`, not compiled by the harness): which draw a warning from gcc at `-Wall -Wextra`, and which flag. Graded against a capture from gcc on Arch; the README says where clang differs |
| `e05-silent-corruption` | PREDICT `[NO-ASAN]` | | an overflow from one struct field into the next, run without sanitizers: predict the printed value of the neighbour, then learn that nothing crashed |
| `e06-fortify-abort` | REPORT `[NO-ASAN]` | | glibc's `*** buffer overflow detected ***: terminated` from a build with `-O2 -D_FORTIFY_SOURCE=2`: what it knows (the destination size at compile time) and what it does not tell you (the line) |

### S06: Comparison

Goal: ordering and equality of strings. `strcmp` compares as `unsigned char` and promises only
the sign; prefixes sort first; `strncmp` and its `n`; case-insensitive comparison; comparators
for `qsort` and `bsearch`, where an array of `char *` and an array of `char[N]` need different
casts; natural ("file9" before "file10") ordering.
Library: `strcmp`, `strncmp`, `memcmp`, `strcasecmp`, `strncasecmp` (`<strings.h>`, POSIX),
`qsort`, `bsearch`.
Prerequisites: S02, S03. 9 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strcmp-strncmp` | WRITE | S | both, comparing as `unsigned char`; the oracle compares signs only, on strings with bytes above 0x7F. `FORBID = strcmp strncmp memcmp` |
| `e02-my-strcasecmp` | WRITE | S | `my_strcasecmp` and `my_strncasecmp` in the "C" locale, with `tolower((unsigned char)c)` |
| `e03-signed-compare` | BUG | | a hand-written compare subtracts `signed char` values, so `"é"` sorts before `"a"` and a binary search misses keys |
| `e04-sort-strings` | WRITE | M | sort and binary-search two tables: an array of `char *` and an array of `char[16]`. Two comparators, two different casts; the lesson explains both, the README states which array is which |
| `e05-comparator-cast` | BUG | | a comparator for an array of `char *` passes its `const void *` arguments straight to `strcmp`, comparing pointer bytes instead of strings |
| `e06-compare-predict` | PREDICT | | the sign of sixteen comparisons: prefixes, `"Z"` against `"a"`, `"10"` against `"9"`, `strncmp` with `n == 0`, differences after an embedded NUL, bytes above 0x7F |
| `e07-natural-order` | WRITE | M | `natural_cmp`: runs of digits compare by numeric value (`isdigit`, no overflow for 40-digit runs), everything else by byte; usable as a `qsort` comparator |
| `e08-prefix-suffix` | WRITE | S | `starts_with`, `ends_with`, `equals_ignore_case`, `common_prefix_len`, each in one pass with `strncmp`/`memcmp`/`strlen` |
| `e09-compare-lookup` | LOOKUP | | `man 3 strcmp`: the exact wording of the return value, and whether it promises -1/0/1 |
| `drills/r01-strcmp-reps` | DRILL | XS | `strcmp`, `strncmp` and case-insensitive compare from memory |
| `drills/r02-comparator-reps` | DRILL | XS | four `qsort` comparators: `char *` elements, `char[N]` elements, a struct's name field, descending case-insensitive |
| `drills/r03-equality-reps` | DRILL | XS | `streq`, `strneq`, prefix, suffix, equal-ignoring-case |

### S07: Character classes

Goal: `<ctype.h>` and its one sharp edge: the argument is an `int` that must be `EOF` or a
value representable as `unsigned char`. Passing a plain `char` holding a byte above 0x7F is
undefined behaviour. The twelve classes and how they nest (`isgraph` ⊂ `isprint`, `isblank` ⊂
`isspace`); `toupper`/`tolower` return `int`; everything depends on the locale and the program
starts in "C"; `EOF` in a `char`.
Library: `isalnum`, `isalpha`, `isblank`, `iscntrl`, `isdigit`, `isgraph`, `islower`,
`isprint`, `ispunct`, `isspace`, `isupper`, `isxdigit`, `tolower`, `toupper`.
Prerequisites: S00, S06. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-eof-in-a-char` | BUG | | a read loop stores `fgetc`'s result in a `signed char` and compares with `EOF`; a 0xFF byte in the input ends the loop early |
| `e02-table-index` | BUG | | a hand-made 256-entry class table indexed by `signed char`: negative index, ASan `global-buffer-overflow`. The same mistake as `isalpha(c)` with a negative `c`, made visible |
| `e03-my-ctype` | WRITE | M | ten `my_is*` functions and `my_toupper`/`my_tolower` for the "C" locale, oracle-tested over every argument from -1 (`EOF`) to 255. `FORBID` lists all of `<ctype.h>` |
| `e04-class-table` | WRITE | S | build a 256-entry bitmask table once (`CLS_DIGIT`, `CLS_ALPHA`, `CLS_SPACE`, `CLS_IDENT`), then classify in one lookup |
| `e05-case-convert` | WRITE | S | `str_upper`, `str_lower`, `str_title`, `str_swapcase` in place; bytes above 0x7F pass through unchanged; every `<ctype.h>` argument is cast correctly |
| `e06-trim` | WRITE | M | `trim_left` (returns a pointer into the argument), `trim_right` (writes a terminator), `trim_view` (pointer and length, nothing modified), `collapse_spaces` in place |
| `e07-ctype-predict` | PREDICT | | a 16 × 12 truth table: sixteen characters (`'\t'`, `' '`, `'_'`, `'0'`, `0x7F`, `'\v'`, ...) against the twelve classes, in the "C" locale |
| `e08-ctype-lookup` | LOOKUP | | `man 3 isalpha` (Linux) / `man 3 ctype` (macOS): the sentence defining the valid argument range, and the return type of `toupper` |
| `drills/r01-classify-reps` | DRILL | XS | all-digits, valid C identifier, blank line, count of each class in a string |
| `drills/r02-case-reps` | DRILL | XS | upper, lower, capitalise first letter, case-insensitive character equality |
| `drills/r03-trim-reps` | DRILL | XS | left, right and both, as a view and in place |

### S08: Searching

Goal: finding characters and substrings. Each function's return on "not found", on the
terminator (`strchr(s, '\0')` is valid and useful), and on an empty needle; the `const` that
`strchr` silently drops; counting and replacing; views into a path.
Library: `strchr`, `strrchr`, `strstr`, `strpbrk`, `memchr`.
Prerequisites: S06. 9 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strchr-strrchr` | WRITE | S | both, including `c == '\0'` and `c` given as an `int` above 127. `FORBID = strchr strrchr memchr` |
| `e02-my-strstr` | WRITE | M | naive `my_strstr`: empty needle returns the haystack, a needle longer than the haystack returns NULL without over-reading. Oracle-tested on a two-letter alphabet so that partial matches are common |
| `e03-my-memmem-strpbrk` | WRITE | M | `my_memmem` (lengths explicit, NUL is an ordinary byte) and `my_strpbrk` |
| `e04-replace-all` | FAULT | M | `str_replace_all(s, from, to)` returning a heap string: count matches, compute the exact size with overflow checks, build in one allocation; `from == ""` is an error; all-or-nothing |
| `e05-const-laundering` | BUG | | `strchr` on a `const char *` returns `char *`; the code writes through it into a string literal and dies. Fix by copying, and say what C23 changes |
| `e06-search-predict` | PREDICT | | the result (an offset or NULL) of sixteen calls: `strchr(s, 0)`, `strstr(s, "")`, `strrchr` on a path with a trailing slash, `strpbrk` with an empty set, `memchr` past a NUL |
| `e07-path-parts` | WRITE | S | `path_base`, `path_ext` and `path_dir_len` returning borrowed pointers and lengths into the argument (`strrchr`), with the contract "valid as long as the argument is" |
| `e08-overlap-trace` | TRACE | | trace the search pointer through a count-occurrences loop for `"aa"` in `"aaaaa"`, advancing by 1 and by the needle length; two different answers |
| `e09-search-lookup` | LOOKUP | | `man 3 strchr`, `man 3 strstr`: the return for a missing character, for the terminator, and for an empty needle |
| `drills/r01-find-reps` | DRILL | XS | index of, last index of, contains, find the nth occurrence |
| `drills/r02-strstr-reps` | DRILL | XS | `strstr` from memory, plus count of non-overlapping occurrences |
| `drills/r03-split-at-reps` | DRILL | XS | split at the first and at the last occurrence of a character into two views, nothing modified |

### S09: Spans and tokens

Goal: `strspn` and `strcspn` as the two primitives behind every scanner; `strtok` and exactly
what it does to its argument (writes NULs, keeps hidden state, merges adjacent delimiters);
`strtok_r`; a `strsep`-style splitter that keeps empty fields; tokenizing without modifying
the input.
Library: `strspn`, `strcspn`, `strtok`, `strtok_r` (POSIX).
Prerequisites: S08; memory M08 for `e08`. 9 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-my-strspn-strcspn` | WRITE | S | both, for sets containing bytes above 0x7F; empty set and empty string. `FORBID = strspn strcspn strpbrk` |
| `e02-chomp-and-skip` | WRITE | S | `chomp` (removes one trailing `"\n"` or `"\r\n"`, safe on the empty string and on a line with no newline), `skip_set`, `span_until`, all one-liners on `strspn`/`strcspn` |
| `e03-my-strtok-r` | WRITE | M | `my_strtok_r` with the POSIX contract, oracle-tested against `strtok_r` token by token, including strings made only of delimiters. `FORBID = strtok strtok_r` |
| `e04-my-strsep` | WRITE | S | `my_strsep(&cursor, delims)`: empty fields are returned, not skipped; `*cursor` becomes NULL after the last field |
| `e05-nested-strtok` | BUG | | an outer `strtok` loop over `;` calls a helper that runs an inner `strtok` loop over `,`; the outer loop ends after one record |
| `e06-view-tokenizer` | WRITE | M | `tok_next(&pos, delims, &start, &len)` over a `const char *`: no writes, no hidden state, restartable |
| `e07-strtok-trace` | TRACE | | draw the bytes of `"a,,b;c"` after each of four `strtok` calls: where each NUL lands, what each call returns, where the saved pointer is |
| `e08-split-under-fire` | FAULT | M | `str_split(s, delim, &count)` returning an owned, NULL-terminated `char **` of owned strings, empty fields kept; all-or-nothing under every `FAIL_AT` |
| `e09-token-lookup` | LOOKUP | | `man 3 strtok`: the sentences about modifying the argument, about static state, and about adjacent delimiters |
| `drills/r01-span-reps` | DRILL | XS | skip a leading set, length of a run, chomp, first-not-of, last-not-of |
| `drills/r02-strtok-r-reps` | DRILL | XS | the word loop: count words, longest word, collect up to N token pointers |
| `drills/r03-field-reps` | DRILL | XS | the nth field of a colon-separated line, empty fields counted, input not modified |

### S10: Numbers from text

Goal: the `strtol` protocol, completely: set `errno = 0`, call, test `end == s` (no digits),
test `errno == ERANGE`, test `*end` (trailing junk), then range-check for the narrower type.
Bases and base 0; leading whitespace and sign; why `strtoul` accepts `"-1"`; `strtod` and what
it accepts (`inf`, `nan`, hex floats); why `atoi` cannot be used on input.
Library: `strtol`, `strtoul`, `strtoll`, `strtoull`, `strtoimax`, `strtoumax`
(`<inttypes.h>`), `strtod`, `strtof`, `atoi`, `atol`, `atof`, `errno`.
Prerequisites: S07, S09. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-parse-int` | WRITE | M | `parse_int(s, &out)` returning `PARSE_OK`, `PARSE_EMPTY`, `PARSE_JUNK` or `PARSE_RANGE`; the whole string must be one decimal `int`; `*out` untouched on error. The harness has 40 inputs, half of them invalid |
| `e02-parse-unsigned` | WRITE | S | `parse_u32` and `parse_size`: reject a leading `-` (which `strtoul` would accept and negate), reject values above the type's maximum |
| `e03-my-strtol` | WRITE | M | `my_strtol` with bases 2-36 and 0, the `0x` prefix rules, clamping to `LONG_MIN`/`LONG_MAX` with `ERANGE`, and `endptr`. Overflow detected before it happens. Oracle-tested against `strtol` on value, `endptr` offset and `errno`. `FORBID` lists the `strto*` and `ato*` families |
| `e04-strtol-predict` | PREDICT | | value, `endptr` offset and `errno` for eighteen calls: `"  42abc"`, `"0x"`, `"0x1F"` in bases 0, 10 and 16, `"08"` in base 0, `""`, `"+"`, twenty nines, `strtoul("-1")` |
| `e05-atoi-accepts` | BUG | | a settings reader uses `atoi`: `"12abc"` becomes 12 and `"abc"` becomes 0, both accepted. Checks fail on the invalid inputs; replace with the full protocol |
| `e06-parse-double` | WRITE | S | `parse_double(s, &out)` via `strtod`: whole string consumed, overflow (`ERANGE` with `HUGE_VAL`) rejected, a stated policy for `inf` and `nan`. Underflow is not tested: its `errno` is implementation-defined |
| `e07-range-list` | FAULT | M | parse `"1,3-7,10"` into a heap array of `int` by walking `endptr`; malformed input and reversed ranges are errors; all-or-nothing |
| `e08-number-lookup` | LOOKUP | | `man 3 strtol`: quote the paragraph that tells the caller to set `errno` to 0 first, and the return values on overflow |
| `drills/r01-strtol-protocol-reps` | DRILL | S | the full protocol four times, for `int`, `long`, `uint8_t` and `int64_t` |
| `drills/r02-digits-reps` | DRILL | XS | by hand: digit value in any base, decimal `unsigned` with overflow check, a hex byte from two characters |
| `drills/r03-endptr-walk-reps` | DRILL | XS | sum whitespace-separated integers, parse `"WxH"`, parse a dotted quad |

### S11: Formatted output

Goal: the `printf` family as a string-producing tool. A conversion specification's five parts
(flags, width, precision, length modifier, conversion); `%.*s` for strings that are not
terminated; the return value of `snprintf` and the two ways it signals trouble (negative; at
least `size`); `sprintf` has no bound; mismatched specifiers are undefined; a format string
from outside the program is a vulnerability; `%n`.
Library: `printf`, `fprintf`, `sprintf`, `snprintf`, `vprintf`, `vfprintf`, `vsnprintf`,
`puts`, `fputs`, `putchar`, `<inttypes.h>` `PRI*` macros, `<stdarg.h>`.
Prerequisites: S04; memory M07. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-format-predict` | PREDICT | | exact output (spaces shown as `·`) and return value of eighteen `snprintf` calls: width and precision on `%s`, `%-8s`, `%08.3f`, `%+d`, `% d`, `%#x`, `%*d`, `%.0f`, `%5c`, `%%`, `%zu` |
| `e02-snprintf-truncation` | WRITE | S | `fmt_into(buf, cap, &len, fmt, ...)` reporting `FMT_OK`, `FMT_TRUNCATED` or `FMT_ERROR`, and `fmt_append` that continues at `len`. Handles `cap == 0` and a negative return |
| `e03-format-from-input` | BUG | | a logger passes the message as the format; a message containing `100%` or `%s` produces garbage or a crash. The check requires the output to equal the message byte for byte |
| `e04-number-to-text` | WRITE | M | `u64_to_str(v, base, buf, cap)` and `i64_to_dec`, correct for 0, `UINT64_MAX` and `INT64_MIN`; oracle is `snprintf` with `PRIu64`/`PRIx64`/`PRId64`. `FORBID` lists the `printf` family |
| `e05-wrong-specifier` | BUG | | a variadic wrapper is called with a `size_t` for `%d` and an `int` for `%ld`; values above 2³² print wrong. (The wrapper has no format attribute, so the compiler cannot warn; the lesson shows what `-Wformat` says when it can) |
| `e06-table-format` | WRITE | M | render rows as an aligned text table into a caller's buffer: column widths computed from the data, text left-aligned with `%-*s`, numbers right-aligned with `%*d`, truncation reported |
| `e07-escape-under-fire` | FAULT | M | `str_escape(bytes, n)` returning a heap string in C-literal form (`\n`, `\"`, `\\`, `\xHH` for the rest): a measuring pass, one allocation, a writing pass; all-or-nothing |
| `e08-printf-lookup` | LOOKUP | | `man 3 printf`: the return value of `snprintf` when the output is truncated, and what the page says about `%n` |
| `drills/r01-snprintf-reps` | DRILL | XS | four bounded formats with truncation detection: an ISO date, `key=value`, a zero-padded id, a hex dump line |
| `drills/r02-digit-reps` | DRILL | XS | unsigned to decimal, hex and binary by hand, digits generated in reverse |
| `drills/r03-pad-reps` | DRILL | XS | pad left, pad right and centre a string in a field with `%*s` and `%-*s` |

### S12: Lines and formatted input

Goal: getting text in. `fgets` (keeps the newline; a line longer than the buffer arrives in
pieces; returns NULL at end of file); `getline` (allocates and grows; returns the length, so
embedded NULs are survivable; the buffer must be freed even after a failure); `sscanf` (the
return value counts assignments; `%s` and `%[` need a width one less than the buffer; `%n`
proves how much was consumed); why `gets` was removed from the language. Harnesses feed input
through `fmemopen`, so nothing depends on files or a terminal.
Library: `fgets`, `fgetc`, `ungetc`, `getline`, `getdelim` (POSIX), `sscanf`, `fscanf`,
`feof`, `ferror`, `fmemopen` (POSIX).
Prerequisites: S09, S10; memory M04. 9 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-read-line` | WRITE | M | `read_line(f, buf, cap, &len)` on `fgets`: returns `RL_OK`, `RL_EOF` or `RL_TOO_LONG`; strips the newline; on an over-long line, discards the rest of that line so the next call starts at the next line; a final line without a newline is a line |
| `e02-getline-loop` | WRITE | S | `count_lines` and `longest_line` with one `getline` buffer reused across iterations and freed exactly once on every path |
| `e03-fgets-predict` | PREDICT | | buffer contents and return value for ten `fgets` calls with an 8-byte buffer over a fixed input: where the newline lands, a line split in two, the last line, the call after end of file |
| `e04-sscanf-records` | WRITE | S | three record parsers on `sscanf`, each proving full consumption with a trailing `%n`: a date `%4d-%2d-%2d`, `name=value` with a bounded scanset, a `%15s %d` pair |
| `e05-unbounded-scan` | BUG | | `sscanf(line, "%s %d", name, &n)` into a 16-byte `name`: ASan `stack-buffer-overflow` on a 40-character token. Fix with a width and say why the width is 15 |
| `e06-sscanf-predict` | PREDICT | | return value and assigned variables for fourteen `sscanf` calls: leading whitespace, a literal that does not match, `%*d`, `%n`, a scanset, empty input (`EOF`) |
| `e07-my-getline` | FAULT | M | `my_getline(&line, &cap, f)` with the POSIX contract, built on `fgetc` and `realloc` with geometric growth; the old buffer survives a failed growth; every `FAIL_AT` |
| `e08-chop-blind` | BUG | | `buf[strlen(buf) - 1] = '\0'` to strip the newline: eats the last character of a final line that has no newline, and writes `buf[-1]` on an empty string |
| `e09-input-lookup` | LOOKUP | | `man 3 fgets`, `man 3 getline`: the return values at end of file, and what the Linux page says about `gets` |
| `drills/r01-fgets-reps` | DRILL | XS | the `fgets` loop: count lines, strip newlines, find the longest, detect a truncated line |
| `drills/r02-sscanf-reps` | DRILL | XS | four record formats, each with a return-value check and a full-consumption check |
| `drills/r03-getline-reps` | DRILL | XS | the `getline` loop: sum one number per line, skip blank lines, free once |

### S13: Heap strings and memory streams

Goal: the library functions that allocate strings for you, and who frees what. `strdup` and
`strndup`; `getline`'s buffer; `open_memstream`, which turns `fprintf` into a growable string
builder; `fmemopen`, which turns a string into a `FILE *`. Assumes M07; does not repeat it.
Library: `strdup`, `strndup`, `open_memstream`, `fmemopen` (all POSIX), `fclose`, `fflush`.
Prerequisites: S04, S11, S12; memory M05, M07. 8 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-dup-family` | WRITE | S | `dup_range(begin, end)`, `dup_trimmed(s)`, `dup_upper(s)` on `strndup`/`strdup`: each returns one owned string or NULL |
| `e02-memstream-builder` | WRITE | M | `render_list(items, n, &len)`: open a memory stream, `fprintf` a numbered list into it, close it, return the buffer. The buffer and size are valid only after `fclose` or `fflush`; the caller frees |
| `e03-fmemopen-reader` | WRITE | S | run the S12 line reader over a string instead of a file: `text_line_count(text)` and `text_nth_line(text, n, buf, cap)` through `fmemopen` |
| `e04-overwrite-leak` | BUG | | a setter assigns a new `strdup` to a field without freeing the old one; the course leak check ends the run with status 4 and names the allocation site |
| `e05-borrow-or-own` | DESIGN | M | a record with an owned `char *name` and a borrowed `const char *label`: write the contract for create, two setters and destroy, then implement them so a failed setter changes nothing |
| `e06-sizes-predict` | PREDICT | | lengths and block sizes for `strdup`, `strndup` with `n` shorter and longer than the source, and the `size` a memory stream reports after a given series of writes |
| `e07-concat-many` | FAULT | M | `str_concat(first, ...)`, variadic and NULL-terminated: one pass over a `va_copy` to measure, one to copy; all-or-nothing |
| `e08-heap-lookup` | LOOKUP | | `man 3 strdup`, `man 3 open_memstream`: who frees, and when the buffer pointer is updated |
| `drills/r01-dup-reps` | DRILL | XS | `my_strdup`, `my_strndup`, duplicate-and-concatenate, duplicate a substring |
| `drills/r02-grow-append-reps` | DRILL | XS | append to a heap string with the `tmp = realloc` pattern, original intact on failure |
| `drills/r03-memstream-reps` | DRILL | XS | open, write, close, use, free: the five steps in order, four times |

### S14: Strings from the system

Goal: the strings a program is handed and the strings it hands back. `argv` (layout, the
`argv[argc] == NULL` guarantee, who owns it); `getenv` (a pointer into the environment: read
it, never write it); `strerror` and `perror` (message text is not portable; `errno` must be
saved before any other call); time as text with `strftime` (returns 0 when the buffer is too
small).
Library: `getenv`, `strerror`, `strerror_r` (POSIX), `perror`, `strsignal` (POSIX),
`strftime`, `gmtime_r` (POSIX).
Prerequisites: S08, S10, S11. 9 exercises, 2 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-argv-parse` | WRITE | M | a hand-written option parser: `-v`, `-n 5`, `-n5`, `--name=value`, `--` ending the options, positional arguments collected in order; errors name the offending argument. The harness passes synthetic `argv` arrays |
| `e02-argv-trace` | TRACE | | draw `argv` for a given command line: the pointer array, its NULL terminator, the strings, and what `argv[1] + 2` and `*++argv` denote |
| `e03-error-message` | WRITE | S | `err_format(buf, cap, context, errnum)` producing `"context: message"`; the harness compares the tail with `strerror(errnum)` rather than with fixed text |
| `e04-errno-clobbered` | BUG | | a failed call is followed by a logging call that itself fails and changes `errno`; the report then names the wrong error. Save `errno` first |
| `e05-env-values` | WRITE | S | `env_get_int(name, fallback, &out)` and `path_search(list, name, buf, cap)` over a colon-separated list, using `getenv`, `strtol` and `strcspn`, never modifying the environment string |
| `e06-getenv-modified` | BUG | | `strtok` run directly on `getenv`'s result: the second lookup of the same variable sees only the first component |
| `e07-timestamps` | WRITE | S | `fmt_iso8601(t, buf, cap)` with `gmtime_r` and `strftime`; a too-small buffer returns an error and leaves an empty string, because `strftime`'s buffer is indeterminate when it returns 0 |
| `e08-strftime-predict` | PREDICT | | output and return value of ten `strftime` calls on a fixed `struct tm` in the "C" locale, including one with a buffer one byte too small |
| `e09-system-lookup` | LOOKUP | | `man 3 strerror`, `man 3 getenv`: what may overwrite the returned string, and what the caller must not do with it |
| `drills/r01-argv-reps` | DRILL | XS | find a flag, find a flag's value, count positionals, join the arguments into a buffer |
| `drills/r02-errno-reps` | DRILL | XS | save `errno`, format an error line into a buffer, restore `errno`: three variants |

### S15: Locales, multibyte and wide characters

Goal: the standard library's own model of text beyond ASCII. A locale decides what a
"character" is; the program starts in "C" until it calls `setlocale`. Multibyte strings
(`char`, variable length), wide strings (`wchar_t`, 32 bits on both course platforms), and
the restartable converters between them with their four kinds of return value. Collation with
`strcoll` and `strxfrm`.
Library: `setlocale`, `localeconv`, `MB_CUR_MAX`, `MB_LEN_MAX`, `mbrtowc`, `wcrtomb`,
`mbrlen`, `mbsinit`, `mbstowcs`, `wcstombs`, `mbsrtowcs`, `wcslen`, `wcscpy`, `wcscmp`,
`wcschr`, `iswalpha`, `iswupper`, `towupper`, `towlower`, `strcoll`, `strxfrm`.
Prerequisites: S06, S07; memory M07. 9 exercises, 3 drills.

Every harness in this unit calls `course_utf8_locale()` (section 9.5) first. Tests that
depend on locale data use only letters whose case mapping is the same in glibc and Apple libc
(Latin-1 Supplement, Greek and Cyrillic basic letters, without `ß` and the Turkish i's).

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-count-characters` | WRITE | S | `mb_count(s)` and `mb_offset(s, k)` with an `mbrtowc` loop and an `mbstate_t`: -1 for an invalid or incomplete sequence |
| `e02-to-wide-under-fire` | FAULT | M | `to_wide(s)` and `to_multibyte(ws)` returning heap strings, sized by a measuring call (`mbstowcs(NULL, s, 0)`); invalid input is an error distinct from allocation failure; all-or-nothing |
| `e03-upper-utf8` | WRITE | M | upper-case a UTF-8 string into a caller's buffer: decode with `mbrtowc`, map with `towupper`, encode with `wcrtomb`; the output may be longer than the input; truncation never splits a character |
| `e04-mbrtowc-predict` | PREDICT | | the return value of fourteen `mbrtowc` calls: ASCII, 2-, 3- and 4-byte sequences, the NUL (0), a sequence cut short by `n` (`(size_t)-2`) and then continued, an invalid lead byte (`(size_t)-1`) |
| `e05-c-locale-surprise` | BUG | | the program never calls `setlocale`; in the "C" locale its character count is wrong (glibc rejects the bytes; Apple libc counts each byte as one character). One fix, both platforms |
| `e06-my-wide-functions` | WRITE | S | `my_wcslen`, `my_wcscmp`, `my_wcschr`, `my_wcscpy`: the `<string.h>` shapes again, one `wchar_t` at a time. Oracle-tested |
| `e07-collation` | WRITE | S | sort strings with `strcoll`, then again by precomputed `strxfrm` keys. The harness checks properties (both sorts agree; in "C", `strcoll` agrees with `strcmp`), never a particular order |
| `e08-split-character` | BUG | | a "truncate to 16 bytes" routine cuts a multibyte character in half; the next `mbrtowc` reports an error. Fix by walking boundaries with `mbrlen` |
| `e09-locale-lookup` | LOOKUP | | `man 3 setlocale`, `man 3 mbrtowc`: the locale at program start, and the four classes of `mbrtowc` return value |
| `drills/r01-mbrtowc-loop-reps` | DRILL | XS | count characters, validate, byte offset of the nth character, widest character in bytes |
| `drills/r02-wide-reps` | DRILL | XS | wide length, copy, compare, find, from memory |
| `drills/r03-convert-reps` | DRILL | XS | measure then convert, both directions, into fixed buffers with truncation reported |

### S16: UTF-8 by hand

Goal: the encoding itself, with no locale and no library. Lead and continuation bytes, the
four lengths, what makes a sequence invalid (overlong forms, surrogates, values above
U+10FFFF, truncation), why `strlen` counts bytes, why byte order equals code point order, and
which `<string.h>` functions therefore still work on UTF-8.
Library: none new. `mbrtowc` is the oracle for well-formed input only; malformed input is
checked against the course's own table, because libcs differ on it.
Prerequisites: S15. 9 exercises, 3 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-utf8-decode` | WRITE | M | `utf8_decode(s, n, &cp)` returning the bytes consumed or 0: rejects overlong forms, surrogates, values above U+10FFFF, stray continuation bytes and truncated sequences |
| `e02-utf8-encode` | WRITE | S | `utf8_encode(cp, out)` returning 1-4, or 0 for a surrogate or an out-of-range value |
| `e03-utf8-validate` | WRITE | S | `utf8_validate(s, n, &bad_offset)`: the offset of the first invalid byte |
| `e04-utf8-navigate` | WRITE | S | count code points, byte offset of the kth, step to the next and to the previous boundary using only the continuation-byte test |
| `e05-overlong` | BUG | | a decoder that accepts `C0 80` and `E0 80 AF`; a path filter then misses an encoded `/`. The classic security defect of UTF-8 decoders |
| `e06-bytes-trace` | TRACE | | write the bits and bytes for six code points, including the boundary pair U+07FF and U+0800 |
| `e07-utf8-truncate` | WRITE | S | truncate in place to at most `n` bytes without splitting a sequence; a buffer of only continuation bytes becomes empty |
| `e08-bytes-not-characters` | PREDICT | | for eight UTF-8 strings: `strlen`, code point count, the sign of `strcmp` against the code point order, what `toupper` does to each byte, whether `strstr` can produce a false match |
| `e09-utf8-lookup` | LOOKUP | | `man 7 utf-8` (Linux) or `man 5 utf8` (macOS): the bit-pattern table and the properties list |
| `drills/r01-lead-byte-reps` | DRILL | XS | sequence length from a lead byte, is-continuation, payload masks |
| `drills/r02-encode-decode-reps` | DRILL | S | both directions from memory |
| `drills/r03-boundary-reps` | DRILL | XS | next boundary, previous boundary, truncate at a boundary |

### S17: Pattern matching

Goal: shell-style patterns and POSIX regular expressions as library calls. A glob matcher
written by hand (the one algorithm in the course); `fnmatch`; the `regcomp` → `regexec` →
`regfree` lifecycle, `regerror`, `regmatch_t` offsets, the -1 offset of a group that did not
take part. POSIX extended syntax only: no `\d`, `\w` or non-greedy operators, which the two
libcs treat differently.
Library: `fnmatch` (`<fnmatch.h>`), `regcomp`, `regexec`, `regerror`, `regfree`
(`<regex.h>`), all POSIX.
Prerequisites: S08, S13. 8 exercises, 2 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-glob-match` | WRITE | M | `glob_match(pattern, s)` for `*`, `?`, `[a-z]`, `[!a-z]` and `\` escapes, iterative with one remembered star position; oracle is `fnmatch(pattern, s, 0)` on generated patterns |
| `e02-glob-blowup` | BUG | | a recursive matcher that takes exponential time on `a*a*a*a*a*a*a*a*b`; correct answers, fails its CPU budget |
| `e03-regex-match` | WRITE | S | `re_is_match(pattern, s, errbuf, cap)`: compile with `REG_EXTENDED | REG_NOSUB`, execute, fill `errbuf` through `regerror` on a bad pattern, `regfree` on every path after a successful compile |
| `e04-regex-captures` | WRITE | M | `re_groups` (group offsets as views) and `re_find_all` (every match in a string, advancing past each one, with `REG_NOTBOL` after the first and no infinite loop on an empty match) |
| `e05-unmatched-group` | BUG | | code copies group 2 without checking `rm_so == -1`; when the optional group does not take part the length is negative and ASan reports `negative-size-param` |
| `e06-regex-predict` | PREDICT | | `rm_so` and `rm_eo` of the whole match for twelve pattern/input pairs: anchors, alternation, bracket expressions, leftmost-longest |
| `e07-grep-under-fire` | FAULT | M | `grep_lines(text, pattern, &count)` returning owned copies of the matching lines; all-or-nothing. (`regcomp` allocates inside libc, where `FAIL_AT` cannot reach; the README says so) |
| `e08-regex-lookup` | LOOKUP | | `man 3 regex` (both platforms): the meaning of `REG_NOSUB`, and of `rm_so` for an unused group |
| `drills/r01-regex-lifecycle-reps` | DRILL | XS | compile, execute, report an error, free: four small matchers |
| `drills/r02-glob-reps` | DRILL | S | the `*` and `?` matcher from memory |

### S18: Bug hunt: the string bug zoo

Goal: one `[BUG]` per class, then mixed files with no hint about which kinds. The README of
`e01`-`e08` names the class; `e09` and `e10` say only how many defects there are.
Library: everything. Prerequisites: S00-S14. 10 exercises, no drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-off-by-one-nul` | BUG | | a stack buffer sized `strlen(s)` instead of `strlen(s) + 1` |
| `e02-truncated-unterminated` | BUG | | `strncpy` at full width, then `%s` |
| `e03-append-overflow` | BUG | | the room check before a `strcat` forgets what is already in the buffer |
| `e04-sizeof-pointer` | BUG | | a capacity computed with `sizeof` on a `char *` (8) instead of on the array it points to |
| `e05-format-injection` | BUG | | user text reaches a format parameter through two layers of wrappers |
| `e06-negative-index` | BUG | | a lookup table indexed by plain `char` data above 0x7F |
| `e07-edits-the-argument` | BUG | | a function documented as read-only tokenizes its `const`-cast argument, and the caller reuses it |
| `e08-static-buffer` | BUG | | a formatter returns a pointer to its `static` buffer; two calls in one `printf` print the same text twice |
| `e09-mixed-bag-a` | BUG | | a record parser with three defects |
| `e10-mixed-bag-b` | BUG | | a report writer with three defects, one of which no tool reports |

### S19: Designing string APIs

Goal: what the library's design teaches by its failures. Every string function should answer
four questions in its contract: who owns the bytes, what the capacity means (with or without
the terminator), whether the result is terminated, and what happens when it does not fit.
String views (pointer plus length) for parsing without copying; fixed-capacity strings; an
owned string with a stored length; C11 Annex K and why nothing implements it.
Library: reuse; `%.*s`.
Prerequisites: S13; memory M05; data structures D20 helps. 7 exercises, 2 drills.

| Folder | Tag | Size | What |
|---|---|---|---|
| `e01-string-view` | WRITE | M | `StrView { const char *p; size_t n; }`: from a C string, equality, prefix test, find, trim, `sv_split_once`, copy out to a terminated buffer. Never reads `p[n]` |
| `e02-view-outlives-buffer` | BUG | | a view taken into a heap string is used after the string is grown with `realloc`: ASan `heap-use-after-free` |
| `e03-fixed-string` | DESIGN | M | `FixStr { char buf[64]; unsigned char len; }`: write the contract for set, append and format (what happens at 63 characters, at 64, at 300), then implement it |
| `e04-contract-review` | PREDICT | | six prototypes from imaginary headers: for each, which of the four questions the prototype and its comment leave unanswered |
| `e05-annex-k-lookup` | LOOKUP | | C11 Annex K (`strcpy_s`): what `__STDC_LIB_EXT1__` is on each course platform (the program prints it), and what the Linux `strcpy` page recommends instead |
| `e06-owned-string` | DESIGN | M | `Str { char *p; size_t len, cap; }` with create, from C string, append, clear, destroy: contract first, including the state after a failed append |
| `e07-str-under-fire` | FAULT | M | `str_append_fmt` and `str_replace` on the owned string from `e06`'s contract (the header is provided, the code is written again); every `FAIL_AT` |
| `drills/r01-view-reps` | DRILL | XS | view from a C string, equality with a C string, trim, split once |
| `drills/r02-bridge-reps` | DRILL | XS | view to terminated buffer with truncation, C string to view, `%.*s` with the `int` cast checked |

### Capstones

| Folder | Builds on | What |
|---|---|---|
| `capstones/c1-mystring` | S01-S09 | `<string.h>` from a blank file: 19 of the 22 C17 functions (all but `strerror`, `strcoll`, `strxfrm`) as `my_*` in one `mystring.c`, each oracle-tested on 100,000 generated cases. `FORBID` lists all of `<string.h>` |
| `capstones/c2-minifmt` | S07, S10, S11 | `mini_vsnprintf`: conversions `d i u x X o c s %`, flags `- 0 + space #`, width and precision including `*`, length modifiers `hh h l ll z`; the `snprintf` return contract exactly; differential-tested against `vsnprintf` on generated formats |
| `capstones/c3-conf` | S09, S10, S12, S13 | an INI reader: sections, `key = value`, comments, quoted values with escapes, typed getters (`int`, `bool`, `double`, string), error messages with line numbers, `getline` input, all-or-nothing load, full fault sweep |
| `capstones/c4-csvstat` | S06, S10, S12, S16 | a CSV reader (quoted fields, doubled quotes, embedded commas and newlines) and a per-column report: count, numeric minimum/maximum/mean via `strtod`, longest value in code points; aligned output that pads by code points, not bytes |

### Totals

| | Count |
|---|---|
| Units | 20 (S00-S19) |
| Exercises | 164 |
| Drills | 51 (each done 3+ times) |
| Capstones | 4 |

## 6. Library coverage matrix

The course's claim is that it covers the standard library's string functions. This table is
how that claim is checked. "Card" is the unit whose lesson holds the function's reference card
(section 8.2). "Written" is where the student reimplements it. `make lint` fails if a function
listed here has no card in the named lesson, or if an exercise table in section 5 names a
library function that is not listed here.

### `<string.h>`, ISO C17: all 22 functions

| Function | Card | Written | Also drilled / used heavily |
|---|---|---|---|
| `strlen` | S01 | S01 e01, C1 | every unit |
| `memcpy` | S02 | S02 e01, C1 | S03, S13 |
| `memmove` | S02 | S02 e01, C1 | S02 r01, S07 e06 |
| `memset` | S02 | S02 e02, C1 | S02 r02 |
| `memcmp` | S02 | S02 e02, C1 | S06 |
| `memchr` | S02 | S02 e02, C1 | S08 |
| `strcpy` | S03 | S03 e01, C1 | S03 r01 |
| `strncpy` | S03 | S03 e02, C1 | S03 r02, S18 e02 |
| `strcat` | S04 | S04 e01, C1 | S04 e03 |
| `strncat` | S04 | S04 e01, C1 | S04 e04 |
| `strcmp` | S06 | S06 e01, C1 | S06 r01, r02 |
| `strncmp` | S06 | S06 e01, C1 | S06 e08, S14 e01 |
| `strcoll` | S15 | | S15 e07 |
| `strxfrm` | S15 | | S15 e07 |
| `strchr` | S08 | S08 e01, C1 | S08 r01 |
| `strrchr` | S08 | S08 e01, C1 | S08 e07 |
| `strstr` | S08 | S08 e02, C1 | S08 e04, r02 |
| `strpbrk` | S08 | S08 e03, C1 | S09 |
| `strspn` | S09 | S09 e01, C1 | S09 r01 |
| `strcspn` | S09 | S09 e01, C1 | S09 e02, S12, S14 e05 |
| `strtok` | S09 | C1 | S09 e05, e07 |
| `strerror` | S14 | | S14 e03, r02 |

### Other ISO C headers

| Header | Functions | Card | Written |
|---|---|---|---|
| `<ctype.h>` | all 12 `is*`, `tolower`, `toupper` | S07 | S07 e03 (12 of 14) |
| `<stdlib.h>` | `strtol`, `strtoul`, `strtoll`, `strtoull`, `strtod`, `strtof` | S10 | `strtol` in S10 e03 |
| `<stdlib.h>` | `atoi`, `atol`, `atof` (card says: not on input) | S10 | |
| `<stdlib.h>` | `getenv`; `qsort`, `bsearch` (as string tools) | S14; S06 | |
| `<stdlib.h>` | `mbstowcs`, `wcstombs`, `MB_CUR_MAX` | S15 | |
| `<inttypes.h>` | `strtoimax`, `strtoumax`, `PRI*` / `SCN*` macros | S10, S11 | |
| `<stdio.h>` | `printf`, `fprintf`, `sprintf`, `snprintf`, `vprintf`, `vfprintf`, `vsnprintf` | S11 | subset in C2 |
| `<stdio.h>` | `puts` | S00 | |
| `<stdio.h>` | `fputs`, `putchar`, `perror` | S11, S14 | |
| `<stdio.h>` | `fgets`, `fgetc`, `ungetc`, `sscanf`, `fscanf`, `feof`, `ferror` | S12 | |
| `<time.h>` | `strftime` | S14 | |
| `<locale.h>` | `setlocale`, `localeconv` | S15 | |
| `<wchar.h>` | `mbrtowc`, `wcrtomb`, `mbrlen`, `mbsinit`, `mbsrtowcs`, `wcslen`, `wcscpy`, `wcscmp`, `wcschr` | S15 | four `wcs*` in S15 e06 |
| `<wctype.h>` | `iswalpha`, `iswupper`, `towupper`, `towlower` | S15 | |

### POSIX.1-2008 (declared under `_POSIX_C_SOURCE 200809L`)

| Header | Functions | Card | Written |
|---|---|---|---|
| `<string.h>` | `strnlen` | S01 | S01 e01 |
| `<string.h>` | `stpcpy`, `stpncpy` | S03 | S03 e01, e02 |
| `<string.h>` | `strtok_r` | S09 | S09 e03 |
| `<string.h>` | `strdup`, `strndup` | S13 | S13 r01 |
| `<string.h>` | `strerror_r`, `strsignal` | S14 | |
| `<strings.h>` | `strcasecmp`, `strncasecmp` | S06 | S06 e02 |
| `<stdio.h>` | `getline`, `getdelim` | S12 | S12 e07 |
| `<stdio.h>` | `open_memstream`, `fmemopen` | S13 | |
| `<time.h>` | `gmtime_r` | S14 | |
| `<fnmatch.h>` | `fnmatch` | S17 | S17 e01 (as `glob_match`) |
| `<regex.h>` | `regcomp`, `regexec`, `regerror`, `regfree` | S17 | |

### Taught but not callable in the course (section 10)

These get a card marked **not in the portable set**, with the reason, and the student writes
the ones that are useful:

| Function | Where it exists | Card | Written |
|---|---|---|---|
| `strlcpy`, `strlcat` | BSD, macOS, glibc 2.38+, not under the course's feature macros | S03, S04 | S03 e03, S04 e02 |
| `strsep` | BSD, glibc | S09 | S09 e04 |
| `memmem`, `strcasestr` | glibc, BSD; `memmem` is in POSIX.1-2024 | S08 | S08 e03 |
| `memccpy` | XSI; C23 | S03 | never (card only) |
| `asprintf` | GNU, BSD | S11 | covered by M07 e04 |
| `gets` | removed in C11 | S12 | never |
| `strcpy_s` and Annex K | optional in C11; neither libc has it | S19 | never |

## 7. Repository layout

Mirrors `LearnCDataStructuresByGrinding`.

```
LearnCStringsByGrinding/
├── README.md               course front page and course map
├── PROGRESS.md             student ledger
├── Makefile                make check | lint | selftest | verify | clean
├── common.mk               shared build rules
├── .gitignore
├── .github/workflows/ci.yml
├── docs/
│   ├── SPEC.md             this file
│   ├── AUTHORING.md        content rules for authors
│   ├── AUTHORING-INFRA.md  build and harness mechanics
│   ├── ROADMAP.md          student-facing learning path
│   ├── FUNCTIONS.md        generated index: every library function → card, exercises, drills
│   │                       (Wave E; not built in Phase 1)
│   └── review/<name>.md
├── lib/
│   ├── course.h            check(), checks_done(), EXIT_OOM, EXIT_LEAK, EXIT_NO_LOCALE
│   ├── bytes.h / bytes.c   byte notation: bytes_notation(), bytes_show()
│   ├── strcheck.h          check_str(), check_bytes(), check_guard(), sign_of()
│   ├── guard.h / guard.c   guarded buffers of an exact capacity
│   ├── oracle.h / oracle.c deterministic string generators for differential tests
│   ├── answers.h / answers.c  the answers.txt grader
│   ├── utf8locale.h / .c   course_utf8_locale() (not locale.h: it would hide <locale.h>)
│   └── dsalloc.h / dsalloc.c  allocation accounting + fault injection (from the DS course)
├── scripts/                verify-all, selftest, lint, fault-sweep, run-timeout,
│                           undef-syms, srchash, rep (from the DS course), forbid.sh;
│                           gen-functions.sh in Wave E
├── units/sNN-<slug>/
├── capstones/cN-<name>/
└── solutions/
```

## 8. File formats: descriptive by design

The earlier courses kept instructions short on purpose: "state the goal, the constraints and
how to verify". That stays true for *how to implement*. It changes for everything else. In
this course:

- **Describe behaviour fully.** What the function does for ordinary input, for each edge case,
  and on failure, in prose and in a table of examples. A student should never have to read the
  harness to learn what is wanted.
- **Describe the library fully.** What the libc function promises, what it requires, what it
  leaves undefined, and which standard says so.
- **Describe the reason.** Why this exercise exists: where the skill is used and which real
  defect it prevents.
- **Describe the checks.** What the harness verifies, in the README, in words.
- **Describe failure.** What a failing run looks like and how to read it.
- **Still never dictate the implementation.** No step lists, no pseudocode of the solution, no
  "first loop over ..., then ...". At most one hint, collapsed. The grind is the student's.

### 8.1 LESSON.md

250-550 lines (S03, with six cards, is about 510). Sections, in this order (headings exactly
as written):

```markdown
# SNN: Title

## Organizer
What the unit is for, what changes in the reader's thinking, prerequisites by unit id, and an
"after this unit you can" list of 4-6 concrete abilities.

## The functions in this unit
| Function | Header | Standard | Card | One-line contract |
One row per library function the unit covers. The Card column says `below`, or names the
unit whose lesson holds the card. This is the map; the cards are the territory.

## Foundations
### <named sections>
The mechanism before the API: what bytes are where, what moves, what is read and written.
Every claim about memory is shown with a byte diagram (section 8.3). Contrasts go in tables
with every cell filled.

## Function reference
### `name`
One card per function (section 8.2).

## Worked example: <title>
Two or three. Each one works a neighbouring problem (never an exercise) from the question to
the bytes: the call, the buffer before, the buffer after, the return value, and what the next
line of code may assume. Each has at least one paragraph beginning
**The plausible wrong reading.**: what a competent reader would conclude, and why it is wrong.

## What the tools say
For each defect the unit teaches: the compiler warning (flag and text) if there is one, the
ASan or UBSan report's first line, and the recognition rule: which token on which line names
the buffer, the size and the access. Quoted only from `lesson-demo/capture*.txt`.

## Distinctions worth keeping straight
| Confusion | What separates them |

## `man` pointers
Exact pages and section names, valid on both platforms; where the page names differ, both.

## Check yourself
6-10 production questions ("what does this return", "write the size expression", "which
byte"), answerable from the lesson alone. Answers in one <details> block.

## Takeaways
4-7 single-sentence claims, each specific enough to be wrong.

## Exercises
| # | Folder | Tag | Size | What |

## Drills
| # | Folder | Skill | Target |
```

Prose rules are those of the data structures `AUTHORING.md` section 3 (one term per concept,
second person, present tense, no filler, never invent tool output), plus:

- **Say what the standard says, and cite it.** "C17 7.24.2.4: if copying takes place between
  objects that overlap, the behavior is undefined." Distinguish *undefined*, *unspecified*,
  *implementation-defined* and *what glibc happens to do*; never present the last as the first.
- **Give every function's history in one sentence when it explains the design.** `strncpy`
  makes sense once you know it was written for 14-byte directory entries.
- **Name the cost.** How many bytes each function reads and writes, and how many passes.
- **A lesson is the reading.** `man` pages are pointers for later, not a substitute. A student
  who reads only the lesson can do every exercise in the unit.

### 8.2 Function reference card

Every library function in section 6 has exactly one card, in the lesson named there. Fixed
fields, in this order; a field that does not apply says "none", it is not dropped.

````markdown
### `strncpy`

```c
#include <string.h>
char *strncpy(char *restrict dst, const char *restrict src, size_t n);
```

| | |
|---|---|
| **Standard** | ISO C89 and later (C17 7.24.2.4) |
| **Does** | Writes exactly `n` bytes to `dst`: the characters of `src` up to and including its terminator, then NUL bytes until `n` bytes have been written. If `src` has `n` or more characters before its terminator, copies the first `n` and writes **no terminator**. |
| **Returns** | `dst`. Never an error indication, never a length. |
| **Requires** | `dst` has room for `n` bytes. `src` is a string, or has at least `n` readable bytes. The two do not overlap. |
| **Undefined if** | The objects overlap; `dst` is smaller than `n`; `src` is neither terminated nor `n` bytes long. |
| **Reads / writes** | Reads at most `min(strlen(src) + 1, n)` bytes. Writes exactly `n`, always. |
| **Terminated afterwards?** | Only if `strlen(src) < n`. |
| **Reports truncation?** | No. |
| **Why it is shaped this way** | It was written to fill fixed-width, NUL-padded fields (file names in early Unix directories), where a full field has no terminator by design. It is not a "safe `strcpy`". |
| **The usual mistake** | `strncpy(dst, src, sizeof dst)` followed by treating `dst` as a string. |
| **Use instead, for strings** | `snprintf(dst, sizeof dst, "%s", src)` and check the return value, or a `strlcpy`-style copy (S03 e03). |
| **Use it for** | Fixed-width fields that are read back with `strnlen` or `%.*s` (S01 e02). |
````

Followed by one byte diagram (section 8.3) for the ordinary case and one for the edge case
that the "usual mistake" row names. Two rows may share a line ("Requires / Undefined if") when
a card would otherwise repeat the previous card's text word for word.

### 8.3 Byte diagrams

Strings are taught by looking at bytes. Lessons, READMEs and `[TRACE]` templates use one
notation:

```
strncpy(dst, "hello", 8)        dst before: # # # # # # # #
index:   0   1   2   3   4   5   6   7
dst:     h   e   l   l   o  \0  \0  \0        terminated: yes   (5 < 8, padded)

strncpy(dst, "hello", 5)
dst:     h   e   l   l   o   #   #   #        terminated: NO    (5 == n)
                             ^ dst[5] was not written
```

Buffers are shown pre-filled with `#` so that unwritten bytes are visible; `\0` is always
written out; a caret line marks pointers. Harness failure messages print buffers in the same
notation (`lib/strcheck.h`), so what the student sees on failure looks like what the lesson
drew.

### 8.4 Exercise README.md

Sections in this order. 60-150 lines for a `[WRITE]`, `[FAULT]` or `[DESIGN]`.

````markdown
# sNN-eNN-<slug> `[TAG]` (SIZE)

## Goal
Two to four sentences: what you build, and what you will be able to do afterwards.

## Background
One to three paragraphs. What the library function (or the idiom) is, where real code uses
it, and the specific defect this exercise trains you to avoid. Points to the lesson section
and the reference card by name. Does not repeat the lesson; orients the exercise in it.

## The API
The prototypes, copied from the header, each followed by its contract in prose: parameters,
what is borrowed and what is owned, the return value, every error case, and what is left
unchanged on failure.

> **This starter does not build yet, on purpose.** ...      (verbatim, section 8.7)

## Behaviour
A table of at least eight concrete cases: the call, the buffer before, the buffer after (in
the byte notation of 8.3), the return value. At least half are edge cases: empty string,
`n == 0`, exact fit, one too many, bytes above 0x7F, no terminator.

## Constraints
Concrete and testable: functions that must not be called (`FORBID`, with the reason), passes
over the input, allocations, bytes that must not be touched, CPU budget.

## What the harness checks
In words, one bullet per group of checks: the fixed cases, the oracle comparison (how many
generated inputs, against which libc function, comparing what), the guard bytes, the timing
budget, the fault sweep.

## Verify
    make check
What success prints. What a failed check looks like, with one sample FAIL line and how to
read it. What ASan output means here.

<details><summary>Hint</summary> at most one; never the algorithm </details>
````

Variants:

- **`[BUG]`** replaces *The API* and *Behaviour* with **Symptom** (what `make check` prints
  now, quoted from a capture), **What the code is supposed to do** (the specification the fix
  must meet), and **What to hand in**: the fix, and `ANSWER.md` with four headings: *The
  defect* (file and line), *The mechanism* (which bytes, which call, why), *How the tool
  showed it* (or why no tool did), *The rule you take away*. The README states how many
  defects there are and promises there are no others.
- **`[REPORT]`** gives the report (`capture.txt`), the source, and a question list answered in
  `answers.txt` before anything is run: the access kind, the size, the buffer's name and size,
  the faulting line, the line that should have prevented it.
- **`[PREDICT]`** explains the setup in full, shows the exact form of an answer with two
  filled-in examples that are not part of the graded set, and lists the keys of `answers.txt`.
- **`[TRACE]`** provides the template in byte notation with the first row filled in.
- **`[LOOKUP]`** names the pages, lists the questions, and asks for the supporting sentence
  from the page, quoted, in `ANSWER.md`. Both platforms' page names are given.
- **`[DESIGN]`** adds **Questions your contract must answer**: the list of situations (a NULL
  argument, a zero capacity, a result that does not fit, a failed allocation) that the header
  comment must settle before any code is written. The harness tests the behaviour the starter
  header fixes; the README says which choices are the student's and which are fixed.

### 8.5 Drill README.md

As in the data structures course (its 7.3), with one addition required by "descriptive": the
**Skill** section says in two or three sentences where this skill returns in the course and in
real code, and the **Functions** section gives each function's contract in one line with one
example. No hints.

### 8.6 Makefile

```make
TAG = WRITE
SRCS = main.c mystrncpy.c
EXPECT_SYMBOLS = my_strncpy my_stpncpy
FORBID = strncpy stpncpy strcpy stpcpy memcpy
STUDENT_SRCS = mystrncpy.c
include ../../../common.mk
```

As in the data structures course (its 7.4), with two new optional variables: `FORBID`
(section 4.6) and `STUDENT_SRCS`, the files `FORBID` applies to (default: every file that
contains the student marker line). `PREDICT`, `LOOKUP` and `REPORT` starters ship `answers.txt`
with every value blank.

| `TAG` (first word) | Starter must | Solution must |
|---|---|---|
| WRITE, DRILL, FAULT, DESIGN | compile, then fail to link with undefined symbols == `EXPECT_SYMBOLS` | pass `make check`; reference no `FORBID` symbol |
| BUG | build, then fail `make check` | pass `make check` |
| REPORT | build; `capture.txt` hash matches source; blank `answers.txt` fails the grader | pass `make check` |
| PREDICT (and LOOKUP) | build, then fail `make check` (blank `answers.txt`) | pass `make check` |
| TRACE | build | pass `make check` (if it has a Makefile) |

### 8.7 Starter sources

As in the data structures course (its 7.5): `main.c` is the self-checking harness; the student
writes below the marker line in `main.c` or in a provided student file; everything above the
marker is byte-identical in the solution. The notice, verbatim, in every WRITE, DRILL, FAULT
and DESIGN README:

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Starter headers are descriptive too: every declared function carries a comment with its full
contract (the same text as the README's *The API* section; `scripts/lint.sh` checks that the
prototypes in the README and the header are identical).

### 8.8 `answers.txt`

```
# s03-e04-strncpy-predict
# One answer per line: key = value. Buffers in byte notation, 8 bytes, no spaces.
call01.buf = hi\0\0\0\0\0\0
call01.ret = dst
call02.buf =
call02.ret =
```

The program computes the true values at run time (it does not embed the answers as text),
reads `answers.txt`, and reports each key as `ok`, `WRONG` or `blank`. It never prints the
correct value. A fact that cannot be computed (which header, which standard) is compared by
a hash of `key=answer`, so the source does not contain it either. Whitespace around `=` is ignored; the README defines the value syntax for the
exercise. A run with any blank or wrong key exits 1.

### 8.9 Solutions

`solutions/<mirror path>/`: every starter file, completed, plus `NOTES.md` with five parts:
**Why it is correct**; **Edge cases and which line handles each**; **Alternatives** (including
the library call that would replace the whole thing, where one exists); **The usual mistake**;
**How libc does it** (one paragraph: the technique real implementations use, such as
word-at-a-time scanning, with no obligation to imitate it). No README links to solutions.

### 8.10 Exit codes

`0` pass; `1` a check failed or an answer is wrong; `3` `EXIT_OOM`; `4` `EXIT_LEAK`; `5`
`EXIT_NO_LOCALE` (no UTF-8 locale found: an environment problem, with instructions on stderr);
`124` time limit; `128+N` killed by signal N.

## 9. Harness design

The harness is the data structures course's, copied at a named commit and extended. Its
sections 8.1-8.9 (commands, isolated builds, the scaffold-OK rule, the solution pass rule,
timeouts, sanitizer configuration per OS, the `dsalloc` leak strategy, nothing skipped
silently, self-tests) apply unchanged. What is new:

### 9.1 Build flags

`-std=c17 -Wall -Wextra -Wpedantic -g`, no optimisation, plus `-U_FORTIFY_SOURCE` so that all
three CI legs run the same code (Ubuntu's gcc otherwise enables fortification when optimising,
and S05 e06 turns it on deliberately through `EXTRA_CFLAGS`). The harness adds `-Werror`.

A `[BUG]`, `[REPORT]` or `[PREDICT]` starter must compile without warnings on gcc, clang and
Apple clang even though it contains a defect. (gcc reports `strncpy` and `snprintf` calls whose
sizes it can see even at `-O0`.) Authors keep the defect out of the compiler's
sight honestly: sizes arrive through function parameters, formats through a wrapper without a
format attribute. They never silence a warning with a pragma or a cast that a student would
have to see through. The lesson's *What the tools say* section shows the warning the compiler
gives when it *can* see the defect, from a capture.

### 9.2 Oracle (differential) testing

`lib/oracle.h` provides a seeded generator (a small LCG; never `rand()`) of strings over a
chosen alphabet, with controls for length distribution, embedded high bytes, and empty
strings. A reimplementation harness runs the student's function and the libc function on the
same inputs and compares:

- **values** exactly, where the standard fixes them (`strlen`, `strspn`, `strtol` with its
  `endptr` offset and `errno`);
- **signs only** for the comparison functions;
- **pointers as offsets** from the argument (`strchr`, `strstr`), with NULL compared as NULL;
- **every byte of the destination and its guards** for the copying functions.

On the first mismatch the harness prints the inputs in byte notation, both results, and the
generator seed and index, so the case can be reproduced; then it stops comparing that
function. Small alphabets (two or three letters) are used wherever partial matches matter.
An oracle is never used where the two libcs may legitimately disagree (section 10).

### 9.3 Forbidden functions

`scripts/forbid.sh` runs `nm -u` on each object built from `STUDENT_SRCS` and fails if a
symbol in `FORBID` is referenced (Apple's leading underscore stripped). Student objects in an
exercise with `FORBID` are compiled with `-fno-builtin`, so a call in the source is a call in
the object and a loop is never turned into a library call. The failure message names the
function and the README section that forbids it. `selftest.sh` proves the check on both
linkers with a fixture that calls a forbidden function directly and through a macro.

### 9.4 Guarded buffers

`lib/guard.h` gives a harness a destination of exactly `cap` bytes:

- in the ASan build, an exact-size heap block, so a write of one byte too many is reported at
  the byte (a zero-capacity buffer is one poisoned byte, reported as `use-after-poison`);
- in the plain build, a block with 32 guard bytes of `0xA5` on each side and the usable region
  pre-filled with `'#'`. `guard_ok()` checks the guards; `check_bytes()` compares the whole
  region, so "wrote the right string but also touched byte 9" fails.

Every harness that tests a function writing into a caller's buffer uses it. `guard_copy(src,
n)` gives a *source* of exactly `n` bytes in the same way, so reading one byte too many (past
a terminator, or `src[n]` of an unterminated field) is caught too.

### 9.5 Locale

`course_utf8_locale()` tries `C.UTF-8` and `en_US.UTF-8` (then `UTF-8` for `LC_CTYPE` only) with
`setlocale(LC_ALL, ...)` and returns the name that worked. If none does, the program prints
how to fix the environment (the `locale-gen` line for Arch) and exits 5. Nothing in the course
calls `setlocale(LC_ALL, "")`: the student's shell settings must not change a result. The
self-test asserts that a UTF-8 locale exists on every CI leg.

### 9.6 Input

Harnesses never read the terminal and never depend on the working directory. Text input is a
string in the harness, opened with `fmemopen`. `[FAULT]` sweeps therefore see the same input on
every run.

### 9.7 What the allocation shim cannot see

`dsalloc` counts and fails the calls made from course code. Memory allocated inside libc
(`getline`, `open_memstream`, `regcomp`, `fmemopen`) is invisible to it: `FAIL_AT` cannot fail
those calls, and on macOS, where there is no LeakSanitizer, a leak of such a block is not
detected. Consequences, all stated in the READMEs concerned:

- a `[FAULT]` exercise that uses one of these functions says which allocations the sweep
  covers and which it does not;
- a `[BUG]` whose defect is a leak leaks a block that course code allocated (S13 e04), so it
  fails on every platform;
- the Linux CI legs are the authority for libc-allocated leaks in solutions.

## 10. Portability rules

Section 9 of the data structures spec applies line by line. Additions for this course:

- [ ] `_POSIX_C_SOURCE 200809L` comes from `dsalloc.h`. Never define `_GNU_SOURCE`,
      `_DEFAULT_SOURCE`, `_BSD_SOURCE`, `_DARWIN_C_SOURCE` or `_XOPEN_SOURCE`.
- [ ] Not callable: `strlcpy`, `strlcat`, `strsep`, `memmem`, `strcasestr`, `memccpy`,
      `memrchr`, `strchrnul`, `mempcpy`, `rawmemchr`, `asprintf`, `vasprintf`, `strverscmp`,
      `strfry`, `strdupa`, `explicit_bzero`, `bzero`, `bcopy`, `index`, `rindex`, `wcwidth`,
      `wcswidth`, `fgetln`, `strtonum`, `<uchar.h>`, Annex K. The student writes the useful
      ones (section 6).
- [ ] Never compare message text from `strerror`, `strsignal`, `regerror` or `perror`. Compare
      with the function's own result, or check that the text is non-empty.
- [ ] Never depend on: the value (as opposed to the sign) of a comparison function; `%s` with
      a NULL argument; `%p` output; `errno` after a successful call; `errno` on `strtod`
      underflow; collation order outside the "C" locale; `mbrtowc` on malformed input or in
      the "C" locale with bytes above 0x7F; `towupper` outside the letter ranges named in S15;
      regex syntax beyond POSIX ERE; whether `char` is signed (use `signed char` or
      `unsigned char` when the exercise is about the sign).
- [ ] `strerror_r` is the XSI version (returns `int`) on both platforms under the course
      macros; say so on its card.
- [ ] `man` page names differ: give both when they do (`isalpha(3)` / `ctype(3)`,
      `utf-8(7)` / `utf8(5)`, `regex(7)` / `re_format(7)`).
- [ ] Tool output in lessons and READMEs comes from a `capture*.txt` made on Arch Linux; say
      so when the macOS output differs in form.

## 11. CI

As in the data structures course: three required legs (`ubuntu-gcc`, `ubuntu-clang`,
`macos-arm64-apple-clang`), each running `make lint`, `make selftest`, `make verify`, then
failing if `git status --porcelain --ignored` shows anything. Lint additionally checks the
lessons (sections, length, a card for every function listed as `below`, tables that name
every folder), README/header prototype agreement (8.7), README section order and minimum
length (8.4), `FORBID` with `STUDENT_SRCS`, and that no course file defines a libc name.
Checking the whole of section 6 against the lessons waits for `gen-functions.sh` (Wave E).

## 12. Authoring

### 12.1 Ownership

The split used for the data structures course (its section 11) carries over: every path has
exactly one owner, and nobody edits a path they do not own.

| Owner | Paths |
|---|---|
| **Monk** (prose) | `units/sNN-*/LESSON.md`; `README.md`; `PROGRESS.md`; `docs/ROADMAP.md`; `docs/review/monk.md` |
| **Jack Gable** (scaffolds) | `units/sNN-*/eNN-*/**`; `units/sNN-*/drills/**`; `units/sNN-*/lesson-demo/**`; `capstones/**`; `docs/review/jack-gable.md` |
| **Bruce** (harness, solutions, review) | `common.mk`; `Makefile`; `.github/**`; `lib/**`; `scripts/**`; `docs/SPEC.md`; `docs/AUTHORING*.md`; `docs/FUNCTIONS.md`; `solutions/**`; all of `units/s03-copying/` (the sample unit), `units/s05-first-reports/` and `units/s18-bug-zoo/` |

Unit folder slugs, fixed now: `s00-bytes-and-nul`, `s01-length`, `s02-mem-functions`,
`s03-copying`, `s04-concatenation`, `s05-first-reports`, `s06-comparison`, `s07-ctype`,
`s08-searching`, `s09-spans-tokens`, `s10-numbers-from-text`, `s11-formatted-output`,
`s12-line-input`, `s13-heap-strings-streams`, `s14-system-strings`,
`s15-locale-multibyte-wide`, `s16-utf8-by-hand`, `s17-pattern-matching`, `s18-bug-zoo`,
`s19-string-api-design`.

### 12.2 Order of work

0. **Phase 1** (Bruce): this spec reviewed and merged; the harness copied and extended
   (`strcheck`, `guard`, `oracle`, `answers`, `locale`, `forbid.sh`, the new lint checks, their
   self-tests); CI green; **sample unit S03 complete** (lesson with its six cards, eight
   exercises, three drills, solutions) as the exemplar for lesson length, card format and
   README depth. Nothing else starts until Nathan has read S03 and agreed that it is
   descriptive enough.
1. **Wave A: S00-S02, S04, S05.**
2. **Wave B: S06-S10.**
3. **Wave C: S11-S14.**
4. **Wave D: S15-S17, S19.**
5. **Wave E: S18 and the capstones;** course map, `PROGRESS.md`, `docs/ROADMAP.md`,
   `docs/FUNCTIONS.md`.

Branch, pull request and review rules are those of the data structures spec, section 11.3 and
11.4. Agents never merge; Nathan merges.

### 12.3 Rules specific to this course

- **A card before a call.** No exercise may require a library function whose card is in a
  later unit. `lint` checks the order against section 6.
- **The README is complete without the harness.** A reviewer who reads only the README must be
  able to predict every check in `main.c`. If a check surprises the reviewer, the README is
  wrong, not the reviewer.
- **No hidden requirements.** If the harness tests it, the README's *Behaviour* table or
  *Constraints* list says it.
- **Reimplementations match libc, including the parts nobody likes** (`strncpy`'s padding,
  `strtok`'s merging of delimiters). The point is to know what the library really does.
- **Every bug is one a tool or a check catches on all three CI legs.** A defect that only
  glibc exposes, or only signed `char` exposes, is rewritten until it is deterministic.
- **No shortcuts**, as before: never tell a student to copy code from an earlier exercise. When
  a later exercise needs an earlier function, the student writes it again or the starter
  provides a different API.

## 13. Open questions

1. **Scope of S15-S17.** Wide characters, UTF-8 and regex are 26 exercises. They are the least
   "standard library `<string.h>`" part of the course. Keep all three, or cut S17 (POSIX, not
   ISO C) to a later course?
2. **Graded predictions.** `answers.txt` makes `[PREDICT]` starters fail `make check`, which
   differs from the earlier courses, where they pass. Accept the difference, or keep the old
   behaviour and grade only on request (`make grade`)?
3. **Lesson length.** 250-550 lines is more than double the earlier courses. The S03 sample
   unit came to about 510 with six cards; if it reads as padded, the range comes down.
4. **Prerequisite.** The spec requires only the memory course. If the data structures course
   should be required too, S01, S09 and S19 can lean on D00, D01 and D20 instead of
   re-explaining.

## 14. Changelog

- 0.3: Wave A built (S00, S01, S02, S04, S05; S03 was Phase 1). Spec changes from building
  it: the `puts` card moves from S11 to S00, the unit where a program first prints a string
  and the only unit whose other library functions (`strlen`, `printf`) are both carded
  elsewhere; S00's lesson also carries a reference card for the `sizeof` operator, which is
  not a library function and so is not in section 6. Two notes that change no rule: S00 e01
  builds its unterminated arrays with braces rather than `char b[3] = "abc"`, because both
  course compilers now reject that form under `-Werror` (the warning itself is quoted in the
  S00 lesson); and S05 e04 ships its ten snippets as one `WARN_SRC` file rather than
  `snippets/*.c.txt`, so that the harness hashes them and `make warn` produces the authority
  for the answers — no capture is shipped, since it would give them away. `common.mk` needed
  no change for S05 e06: `EXTRA_CFLAGS` is placed after `-U_FORTIFY_SOURCE`, so
  `-O2 -D_FORTIFY_SOURCE=2` takes effect, which was confirmed by observing the abort fire and
  by checking that it does not without either flag.
- 0.2: Phase 1 built. Harness copied from the data structures course and extended (`bytes`,
  `strcheck`, `guard`, `oracle`, `answers`, `utf8locale`, `forbid.sh`, `FORBID`/`STUDENT_SRCS`,
  `-U_FORTIFY_SOURCE`, multi-file captures including compiler warnings, graded `answers.txt`
  starters, lint for README and lesson structure, 18 new self-tests). Sample unit S03
  complete. Spec changes from building it: lesson length 250-550; `lib/locale.h` renamed
  `utf8locale.h`; a Card column in the lesson's function table; hashed answers for facts that
  cannot be computed; `guard_copy` for sources; e08 widened to nine facts.
- 0.1: first draft. Curriculum (20 units, 164 exercises, 51 drills, 4 capstones), library
  coverage matrix, descriptive lesson and README formats, function reference card, oracle
  testing, graded predictions, harness extensions.
