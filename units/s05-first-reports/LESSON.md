# S05: Your first string-bug reports

## Organizer

Four tools can tell you that a string operation went wrong, and they tell you different
things, at different times, in different words. This unit is six exercises of reading what
they actually print — early, because every unit from here on produces these reports, and a
report you cannot read is a report you will work around instead of fixing.

What changes in your thinking: you stop reading a report as "the program crashed here" and
start reading it as **four facts and a question**. The facts are the kind of storage, the
kind of access, how much, and which object. The question is always the same and the report
never answers it: *which line should be different?* For a write overflow the two are usually
close together; for a read overflow they are in different functions, and for the two defects
in this unit that no tool reports at all, there is no report to start from.

You need S04 (and through it S00-S03): the reports here are of copies and appends. The
memory course's M02b is the equivalent unit for allocation bugs; this one is its string half.

After this unit you can:

- name the storage and the access kind from a report's first two lines, and say which block
  at the bottom names the object;
- find the declaration behind a stack, heap or global overflow in under a minute;
- tell a "wrote too much" defect from a "read without a terminator" one, and say which end of
  the program each sends you to;
- say which frame in a report is yours, however many library frames sit above it;
- predict whether a compiler can see a given string defect, and say what hides it;
- recognise glibc's `*** buffer overflow detected ***` and know exactly how little it says;
- name the one overflow AddressSanitizer cannot report, and why.

## The functions in this unit

| Function | Header | Standard | Card | One-line contract |
|---|---|---|---|---|
| — | — | — | — | No new library functions: this unit is about tools |

Everything here uses `strcpy`, `strcat`, `strlen`, `memcpy`, `snprintf` and `printf`, whose
cards are in S00-S04 and S11. What is new is `_FORTIFY_SOURCE`, which is a build setting
rather than a function, and the four diagnostics below.

## Foundations

### The four tools, and what each one can see

| Tool | Runs | Needs | Sees | Says |
|---|---|---|---|---|
| **compiler warning** | at build time | both sizes as compile-time constants | the call it can fold | file, line, flag, and often the object's declaration |
| **`_FORTIFY_SOURCE`** | at run time | `-O1`+ and a destination whose size the optimiser knows | overflows of those destinations | one line, no names |
| **AddressSanitizer** | at run time | a build with `-fsanitize=address` | any access that leaves an **object** | kind, access, size, object, frames, allocation site |
| **a check in the program** | whenever you run it | that you thought of it | whatever you compare | whatever you print |

Read the "Needs" column as a list of ways to hide a defect. Pass a size through a parameter
and the compiler goes quiet. Build without `-O2` and fortification disappears. Keep the
overflow inside one object and ASan has nothing to report. Write no test for the neighbouring
field and the fourth row is empty too. Every exercise in this unit is one row of that table,
and e04 and e05 are the two where the tools lose.

### The anatomy of an AddressSanitizer report

Every ASan report has the same five parts, and three of them are always in the same place:

```
==PID==ERROR: AddressSanitizer: <KIND> on address 0x... at pc 0x... bp 0x... sp 0x...   <- 1
READ|WRITE of size N at 0x... thread T0                                                 <- 2
    #0 ... (/usr/lib/libasan.so.8+0x...)                                                <- 3
    #1 ... in your_function .../your_file.c:LINE
<a block describing the object>                                                         <- 4
SUMMARY: AddressSanitizer: <KIND> .../file.c:LINE in function                           <- 5
```

1. **The kind** — `stack-buffer-overflow`, `heap-buffer-overflow`,
   `global-buffer-overflow`, `memcpy-param-overlap`, `SEGV on unknown address` — which tells
   you which storage and therefore which shape part 4 will have.
2. **The access**: `READ` or `WRITE`, and how many bytes *that operation* was about to touch.
   For an intercepted string function this is the whole range, not one machine access: a
   `strcpy` of a 12-character name reports 13.
3. **The frames.** Frame `#0` is inside `libasan`, because ASan replaces about forty string
   and memory functions with checking versions. There may be **more than one** library frame
   — a `printf` report has three — so the rule is "the first frame whose file is one of
   yours", not "frame #1".
4. **The object**, in one of three shapes:
   - *stack*: a frame table listing each local with its byte range, name and declaration line;
   - *heap*: `is located N bytes after an M-byte region`, then an `allocated by` stack;
   - *global*: `is located ... global variable 'name' defined in 'file.c:line:col' of size N`.
5. The summary, which repeats the first frame.

The useful habit is to read 1, 2, 4, 3 — kind, access, object, then the frame — because by
the time you reach the frames you already know what you are looking for.

### READ or WRITE: the most informative word in the report

| | What it means | Where the defect is |
|---|---|---|
| **WRITE** overflow | a copy that did not fit | at or near the reported line: the call, or the buffer's size |
| **READ** overflow | a reader handed bytes that are not a string | **somewhere else**: the copy or allocation that left no terminator |

For string code this distinction is almost a diagnosis. A WRITE report points at a `strcpy`,
a `strcat` or a `memcpy` with the wrong size, and the fix is usually on the line named. A
READ report points at an innocent `strlen`, `strcmp` or `printf("%s")` that was handed a
character sequence, and the fix is wherever those bytes were produced — often in another
file, reached through the report's `allocated by` stack if the bytes are on the heap.

e01 is a WRITE and e03 is a READ, deliberately, of programs with the same shape.

### What the compiler can see

A compiler diagnoses a string bug only when both ends are constants it can fold. From
`lesson-demo/capture-warn.txt`, gcc at `-O2 -Wall -Wextra -Wpedantic` on four defects:

```
warn.c: In function ‘count’:
warn.c:19:21: warning: format ‘%d’ expects argument of type ‘int’, but argument 2 has type ‘size_t’ {aka ‘long unsigned int’} [-Wformat=]
   19 |     return printf("%d items\n", n);
      |                    ~^           ~
      |                     |           |
      |                     int         size_t {aka long unsigned int}
      |                    %ld
warn.c: In function ‘clear’:
warn.c:25:25: warning: argument to ‘sizeof’ in ‘memset’ call is the same expression as the destination; did you mean to provide an explicit length? [-Wsizeof-pointer-memaccess]
   25 |     memset(p, 0, sizeof p);
      |                         ^
warn.c: In function ‘greet’:
warn.c:12:5: warning: ‘__builtin_memcpy’ forming offset [8, 22] is out of the bounds [0, 8] of object ‘small’ with type ‘char[8]’ [-Warray-bounds=]
   12 |     strcpy(small, "a much longer greeting");
      |     ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
warn.c:10:10: note: ‘small’ declared here
   10 |     char small[8];
      |          ^~~~~
warn.c: In function ‘label’:
warn.c:33:39: warning: ‘%s’ directive output truncated writing 10 bytes into a region of size 8 [-Wformat-truncation=]
   33 |     return snprintf(buf, sizeof buf, "%s", "abcdefghij");
      |                                       ^~   ~~~~~~~~~~~~
warn.c:33:12: note: ‘snprintf’ output 11 bytes into a destination of size 8
   33 |     return snprintf(buf, sizeof buf, "%s", "abcdefghij");
      |            ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

Four for four, each naming the flag and, where it helps, the object's declaration. Now the
same file through clang, from `capture-warn-clang.txt`:

```
warn.c:12:5: warning: 'strcpy' will always overflow; destination buffer has size 8, but the source string has length 23 (including NUL byte) [-Wfortify-source]
   12 |     strcpy(small, "a much longer greeting");
      |     ^
warn.c:19:33: warning: format specifies type 'int' but the argument has type 'size_t' (aka 'unsigned long') [-Wformat]
   19 |     return printf("%d items\n", n);
      |                    ~~           ^
      |                    %zu
warn.c:25:25: warning: 'memset' call operates on objects of type 'char' while the size is based on a different type 'char *' [-Wsizeof-pointer-memaccess]
   25 |     memset(p, 0, sizeof p);
      |            ~            ^
warn.c:25:25: note: did you mean to provide an explicit length?
   25 |     memset(p, 0, sizeof p);
      |                         ^
```

Three of four, in different words, one of them under a flag gcc does not have
(`-Wfortify-source`, which comes from glibc's header annotations rather than from a pure
compiler analysis). Neither compiler is more correct; a project that builds with both gets
the union, which is why this course's CI runs gcc and clang.

And what hides a diagnostic? One parameter. `memset(p, 0, sizeof p)` warns; the same call
inside a helper whose size is a parameter does not (S02 e04). `strncat(buf, s, sizeof buf)`
warns; behind a helper it does not (S04 e04). e04 of this unit is ten snippets, five of which
hide exactly one constant, and the exercise is predicting which five.

### What the compiler cannot see, and libc can

`_FORTIFY_SOURCE` is the middle ground: the destination's size has to be known at compile
time, but the *source's length* does not. glibc's headers redirect `strcpy` to
`__strcpy_chk(dst, src, __builtin_object_size(dst, 0))`, which compares at run time and calls
`abort()` if the copy would not fit.

```
*** buffer overflow detected ***: terminated
```

That is the entire message — no file, no line, no buffer, no size. It costs a comparison per
call, which is why every mainstream distribution builds its packages with it, and it tells
you almost nothing, which is why you then rebuild with ASan. It also needs **both** the macro
and optimisation: without `-O2`, `__builtin_object_size` returns "unknown" and the check
compiles away. e06 is that program, with both experiments.

### What nothing can see

Two defects in this unit produce no diagnostic from any tool, in any build.

**An overflow inside one object.** ASan surrounds each *object* with redzones, and a struct
is one object:

```c
typedef struct { char user[8]; char role[8]; } Account;   /* 16 bytes, one object */
```

A 13-character name copied into `user` writes 14 bytes, all of them inside the struct. No
redzone is touched. From `lesson-demo/capture-quiet.txt`, the plain build, with no sanitizer
and nothing to report:

```
before: \0\0\0\0\0\0\0\0guest\0\0\0
after:  mallory_admin\0\0\0
user:   mallory_admin
role:   admin
is admin: yes
```

The name was chosen by someone who had read the struct definition: eight characters to fill
`user`, then `admin`, then the terminator `strcpy` supplies free. `role` now holds `admin`
and the program agrees that the account is an administrator. e05 is that program, and the
only thing that catches it is a check on the neighbour's value.

**A conversion that loses information.** S01 e06's one-byte length field is the other: a
`size_t` stored in an `unsigned char` is a defined, silent, lossy conversion. No sanitizer
reports it, and `-Wconversion` is far too noisy to leave on.

The rule both of them teach: **a clean sanitizer run means "no access left its object", not
"the program is correct".**

## Function reference

This unit introduces no library functions. What it introduces instead is one build setting,
documented here in the same shape as a function card because it behaves like one.

### `_FORTIFY_SOURCE`

```c
#define _FORTIFY_SOURCE 2      /* or on the command line: -D_FORTIFY_SOURCE=2 */
#include <string.h>            /* the redirection happens in the header */
```

| | |
|---|---|
| **Standard** | None. A glibc extension, also implemented by the BSDs and by Apple's libc in a reduced form. Levels 1, 2 and (glibc 2.35+) 3. |
| **Does** | Makes glibc's headers redirect about twenty string and memory functions to checking variants (`__strcpy_chk`, `__memcpy_chk`, `__snprintf_chk`, ...) that take the destination's size as computed by `__builtin_object_size`. At run time, a call that would overflow calls `__chk_fail`, which prints one line and aborts. |
| **Returns** | none. The program does not continue. |
| **Requires** | Optimisation (`-O1` or better): without it the compiler cannot compute the object size, and the checks compile away. The destination's size must be knowable — a local array, a struct member, a `malloc` with a constant size. |
| **Undefined if** | none: it only adds checks. |
| **Reads / writes** | One comparison per redirected call. The cost is small enough that distributions enable it by default. |
| **Terminated afterwards?** | none. |
| **Reports truncation?** | No: it reports overflow, by aborting. |
| **Why it is shaped this way** | It was designed to be free enough for production. The message says nothing because it runs on users' machines, where a detailed message would be an information leak; the details are in the core dump. |
| **The usual mistake** | Assuming a program is checked because the macro is defined, when the build has no optimisation. Second: expecting the message to name the buffer. |
| **Use instead** | AddressSanitizer while developing, which names everything; this in the shipped build, which costs nothing. |
| **Use it for** | Release builds. This course's normal build passes `-U_FORTIFY_SOURCE` so that every CI leg runs the same libc entry points (SPEC 9.1); S05 e06 turns it back on through `EXTRA_CFLAGS`. |

## Worked example: reading a stack report backwards

From `lesson-demo/capture-stack.txt`, a 12-character name copied into an 8-byte local:

```
==2096821==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7a924e6d0048 at pc 0x7e92516bc4ce bp 0x7fff4cc024b0 sp 0x7fff4cc01c68
WRITE of size 13 at 0x7a924e6d0048 thread T0
    #0 0x7e92516bc4cd  (/usr/lib/libasan.so.8+0xbc4cd)
    #1 0x5652720e0511 in copy_into .../lesson-demo/main.c:26
    #2 0x5652720e07bf in main .../lesson-demo/main.c:46

Address 0x7a924e6d0048 is located in stack of thread T0 at offset 72 in frame
    #0 0x5652720e062c in main .../lesson-demo/main.c:40

  This frame has 2 object(s):
    [32, 48) 'a' (line 67)
    [64, 72) 'tag' (line 44) <== Memory access at offset 72 overflows this variable
```

Read it in the order that answers questions fastest.

**The kind** is `stack-buffer-overflow`, so the object will be described by a frame table at
the end, and there is no allocation stack to look for.

**The access** is `WRITE of size 13`. Thirteen, not twelve: `strcpy` copies the terminator
too. And a WRITE means a copy that did not fit, so the defect is at or near the line that
made it.

**The object** is in the frame table. This frame has two locals, and the one marked with
`<==` is `'tag'`, bytes `[64, 72)` — eight bytes — declared on the line the table gives.
Subtract: 13 written, 8 inside, so **five bytes landed outside**. The two numbers answer
different questions — 13 points at the source, 5 at the damage.

**The frame** is `#1`, because `#0` is inside `libasan`. With clang frame `#0` is named
`in strcpy`; with gcc it is an address and a library path. Either way the first frame with
one of your files in it is the call.

One line in the report is a trap: the block that begins `Address ... is located in stack of
thread T0 ... in frame #0 ... main.c:40` gives the line the *frame's function* starts on, not
the line of the access. The access is in the frame list at the top, at `main.c:26`.

**The plausible wrong reading.** "`WRITE of size 13` means thirteen bytes were written out of
bounds." It means the operation was about to write thirteen bytes in total, and ASan checks
the whole range before performing it. How many of them are outside is `13 - 8 = 5`, which the
report does not state. Getting this wrong matters when you use the size to find the source:
a report of `WRITE of size 13` tells you to look for a twelve-character string, and five
would send you looking for the wrong one.

## Worked example: the same program, read instead of written

From `lesson-demo/capture-heap.txt`, a six-character field copied into a six-byte heap block
and then printed:

```
==2096838==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x7a2df30e0016 at pc 0x7e0df4e76d36 bp 0x7ffd15180790 sp 0x7ffd1517ff48
READ of size 7 at 0x7a2df30e0016 thread T0
    #0 0x7e0df4e76d35  (/usr/lib/libasan.so.8+0x76d35)
    #1 0x5faf84d33894 in main .../lesson-demo/main.c:56
    #2 0x7e0df4227780  (/usr/lib/libc.so.6+0x27780)
    #3 0x7e0df42278b8 in __libc_start_main (/usr/lib/libc.so.6+0x278b8)

0x7a2df30e0016 is located 0 bytes after 6-byte region [0x7a2df30e0010,0x7a2df30e0016)
allocated by thread T0 here:
    #0 0x7e0df4f2c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1)
    #1 0x5faf84d365a3 in ds_malloc_at .../lib/dsalloc.c:216
    #2 0x5faf84d3382e in main .../lesson-demo/main.c:51
    #3 0x7e0df4227780  (/usr/lib/libc.so.6+0x27780)
```

Same tool, same shape of program, four differences.

The **kind** is `heap-`, so there is an `allocated by` stack rather than a frame table. The
**access** is a `READ`, so the reader was handed bytes that are not a string. The **object**
block gives the block's size (6) and says the bad byte is `0 bytes after` it — and `READ of
size 7` is exactly one more than 6, the signature of a missing terminator rather than a
missing bounds check.

And the **frame to read is in the other stack**. The access's frames are ASan's checker and
then `main`, both innocent — `main` only printed. The line to change is in the `allocated by`
stack, past `malloc` and the course's `ds_malloc_at`, at the call that asked for the field's
width instead of the field's width plus one. (In e03 the same report has three `libasan`
frames above `main`, because there the `printf` goes through ASan's `printf` and `vprintf`
interceptors; how many library frames sit on top varies, and the rule does not.)

**The plausible wrong reading.** "The report names `main.c` and a line, so that is where to
fix it." For a READ overflow it almost never is. The reader is doing what `%s` is specified
to do; the bytes were wrong before it ever saw them. Fixing the reader — printing with
`%.*s`, or measuring with `strnlen` — silences this report and leaves every other reader of
that block broken. The habit to build: on a READ report, scroll **past** the frame list to
the allocation stack, and start there.

## What the tools say

This whole unit is that section, so here is the summary table instead, with the capture each
line comes from.

| Report | Looks like | From |
|---|---|---|
| stack overflow | `stack-buffer-overflow`, a frame table with `[32, 40) 'tag' (line 25)` | e01, `capture-stack.txt` |
| heap overflow | `heap-buffer-overflow`, `0 bytes after 6-byte region`, an `allocated by` stack | e03, `capture-heap.txt` |
| global overflow | `global-buffer-overflow`, `is located ... global variable 'months' defined in 'table.c:7:19' of size 48` | e02, `capture-global.txt` |
| overlap | `memcpy-param-overlap`, two address ranges and no object at all | S02 e03 |
| a write to a literal | `SEGV on unknown address`, `caused by a WRITE memory access`, no object | S00 e03 |
| a compiler warning | a file, a line, a flag in brackets, often a `note:` with the declaration | e04, `capture-warn.txt` |
| fortification | `*** buffer overflow detected ***: terminated`, and nothing else | e06 |
| an overflow inside one object | nothing at all | e05, `capture-quiet.txt` |
| a lossy conversion | nothing at all | S01 e06 |

From `lesson-demo/capture-global.txt`, the global case, which is the shortest of the three
ASan shapes:

```
==2096855==ERROR: AddressSanitizer: global-buffer-overflow on address 0x5e9400ebf02c at pc 0x71f804876d36 bp 0x7fff44faae70 sp 0x7fff44faa628
READ of size 1 at 0x5e9400ebf02c thread T0
    #0 0x71f804876d35  (/usr/lib/libasan.so.8+0x76d35)
    #1 0x5e9400eb796d in main .../lesson-demo/main.c:63
    #2 0x71f803c27780  (/usr/lib/libc.so.6+0x27780)
    #3 0x71f803c278b8 in __libc_start_main (/usr/lib/libc.so.6+0x278b8)
    #4 0x5e9400eb73e4 in _start (.../lesson-demo/build/asan/prog+0xc3e4)

0x5e9400ebf02c is located 52 bytes before global variable '*.LC0' defined in 'main.c' (0x5e9400ebf060) of size 7
  '*.LC0' is ascii string 'main.c'
0x5e9400ebf02c is located 0 bytes after global variable 'months' defined in 'main.c:16:19' (0x5e9400ebf020) of size 12
```

Note the two `is located` lines. ASan describes the address relative to the nearest globals
it knows, and the compiler emits globals you never wrote — string literals appear under names
like `*.LC0`. Read them all and take the one that names something from your source.

## Distinctions worth keeping straight

| Confusion | What separates them |
|---|---|
| the faulting line vs. the line to change | For a WRITE they are usually the same; for a READ the second is wherever the terminator went missing. |
| frame `#0` vs. the first frame that is yours | `#0` is ASan's interceptor. A `printf` report has three library frames before yours. |
| the frame list's line vs. the "located in stack ... in frame" line | The first is the access; the second is where the frame's function begins. |
| `access.size` vs. bytes out of bounds | The report gives the whole operation's range. The overflow is that minus the object's size. |
| a named global vs. `*.LC0` | The second is a compiler-generated literal. Only a name from your source is useful. |
| ASan silent vs. the program correct | ASan reports accesses that leave an **object**. A struct is one object. |
| a compiler warning vs. no defect | A warning needs both sizes as constants. One parameter hides any of them. |
| `_FORTIFY_SOURCE` defined vs. in effect | Without `-O1` or better, the checks compile away. |
| 128 + N vs. N | A shell reports a signal-killed process as 128 + N: `SIGABRT` is 6, so the status is 134. |

## `man` pointers

- `man 7 feature_test_macros` (Linux only): what `_FORTIFY_SOURCE` and its relatives do, and
  the rule that they must be defined before any header is included.
- `man 3 strcpy`, `man 3 strcat`: the CAVEATS sections, which say what the functions require
  of the caller — the requirements these reports are about.
- `man 7 signal` (Linux) / `man 3 signal` (macOS): `SIGABRT` is 6 and `SIGSEGV` is 11, which
  is where 134 and 139 come from.
- `gcc(1)`, the *Warning Options* section: `-Wstringop-overflow`, `-Wstringop-truncation`,
  `-Wstringop-overread`, `-Warray-bounds`, `-Wformat`, `-Wformat-truncation`,
  `-Wsizeof-pointer-memaccess`. Worth one read through, once.
- The AddressSanitizer documentation is not a manual page; `ASAN_OPTIONS=help=1 ./prog`
  prints every run-time option the build supports.

## Check yourself

1. A report says `READ of size 9` and `0 bytes after 8-byte region`, with `strlen` above your
   frame. What happened, and which line do you change?
2. Where in a stack report is the variable's declaration line, and where is the faulting
   line?
3. Why does a `printf("%s")` report have three library frames when a `strcpy` report has one?
4. A 16-byte struct of two 8-byte char arrays is overflowed from the first into the second.
   What does ASan report?
5. `*** buffer overflow detected ***` and exit status 134. Which tool stopped the program,
   and what are the two build settings it needed?
6. gcc warns about `memset(p, 0, sizeof p)` but not about `return sizeof p`. Why?
7. A report's second line says `WRITE of size 13` and the object is 8 bytes. How many bytes
   were written out of bounds, and how long was the source string?
8. You have a `global-buffer-overflow` whose "is located" line names `*.LC0`. What do you do
   next?

<details><summary>Answers</summary>

1. Something asked for the length of an 8-byte block holding eight characters and no
   terminator. The line to change is not the `strlen`: it is the copy or allocation that
   produced eight bytes where nine were needed.
2. The declaration line is in the frame's object table (`[32, 40) 'tag' (line 25)`); the
   faulting line is in the frame list at the top, in the first frame that is yours.
3. Because ASan intercepts `printf`, which calls its `vprintf`, which calls the shadow check:
   three frames in `libasan` before yours. `strcpy`'s interceptor checks directly.
4. Nothing. Every byte written is inside one object, so no redzone is touched.
5. glibc, at run time. It needed `-D_FORTIFY_SOURCE=1` or more **and** optimisation, because
   the destination's size comes from `__builtin_object_size`.
6. The first *uses* the value as a byte count in a function gcc has a pattern for; the second
   is an ordinary expression whose misuse would be in the caller.
7. Five bytes out of bounds (13 − 8), and the source was twelve characters:
   `strcpy` writes `strlen(src) + 1`.
8. Read the other "is located" lines. `*.LC0` is a compiler-generated string literal; the
   useful line names a variable from your source, with its file, line and size.

</details>

## Takeaways

- An ASan report is four facts — kind, access, size, object — and the question it never
  answers: which line should be different.
- `READ` means a reader was handed something that is not a string, and the line to change is
  where those bytes were produced.
- Frame `#0` is always ASan's; the frame to read is the first one in a file of yours, and
  there may be three library frames above it.
- The access size is the whole operation's range, not the part that was out of bounds.
- A compiler warning needs both sizes as compile-time constants, and one parameter hides any
  of them.
- `_FORTIFY_SOURCE` catches overflows of destinations whose size the optimiser knows, costs
  almost nothing, and tells you almost nothing.
- A clean sanitizer run means no access left its object: an overflow between two members of
  one struct is invisible to every tool, and only a check on the neighbour finds it.

## Exercises

| # | Folder | Tag | Size | What |
|---|---|---|---|---|
| e01 | `e01-stack-overflow-report/` | `[REPORT]` | | a `stack-buffer-overflow` WRITE: the frame table, the sizes, the overflow |
| e02 | `e02-global-overflow-report/` | `[REPORT]` | | a `global-buffer-overflow`: which "is located" line is the useful one |
| e03 | `e03-overread-report/` | `[REPORT]` | | a READ inside `printf`: the allocation stack, and the line that is not named |
| e04 | `e04-compiler-warnings/` | `[PREDICT]` | | ten snippets: which ones gcc warns about, and under which flag |
| e05 | `e05-silent-corruption/` | `[PREDICT]` `[NO-ASAN]` | | an overflow inside one struct, with every tool silent |
| e06 | `e06-fortify-abort/` | `[REPORT]` `[NO-ASAN]` | | glibc's `*** buffer overflow detected ***`, and what it does not say |

## Drills

This unit has no drills: there is nothing to make automatic, only reports to read. The
reading returns in every `[BUG]` and `[REPORT]` exercise from S06 onwards, and in the bug zoo
of S18.
