# s05-e06-fortify-abort: notes

## Why each answer is what it is

| Key | Answer | Reason |
|---|---|---|
| `buffer.size` | `16` | `GREET_SIZE` in `greet.h` |
| `fits.longest` | `7` | the greeting adds `"Hello, "` (7) and `"!"` (1), and one byte is the terminator: 16 − 1 − 8 |
| `stopped_by` | `libc` | the message comes from glibc's `__chk_fail`, not from the compiler and not from a sanitizer |
| `exit.signal` | `SIGABRT` | the check calls `abort()` |
| `exit.status` | `134` | 128 + 6, the shell's convention for "killed by signal 6" |
| `names.line` | `no` | the message contains no file, line or function |
| `names.buffer` | `no` | nor any identifier at all |
| `asan_would_say` | `stack-buffer-overflow` | the buffer is a local array in `greet_print` |

`"Hello, Bartholomew!"` is nineteen characters and needs twenty bytes; the buffer has
sixteen. The longest name that fits is seven characters — `"Hello, Ada!"` at eleven
characters fits comfortably, and `"Hello, Bartholo!"` at sixteen characters would need
seventeen bytes.

## What the message does and does not tell you

```
*** buffer overflow detected ***: terminated
```

That is the entire report. Compare it with the three ASan reports in this unit, which name
the variable, its size, its declaration line, the faulting line and (for the heap) the
allocation site. The fortify check knows the destination's size — it was passed as an extra
argument — and does not print it; it knows which function it is in and does not print that
either. glibc's reasoning is that this check runs in production, where a detailed message is
an information leak and a core dump is the right place for the details.

So what do you do with it? Three things, in order:

1. **The core dump or a debugger.** `gdb ./prog core` and a backtrace names the function —
   typically `__strcat_chk`, with your frame below it.
2. **Rebuild with ASan.** The same input under `-fsanitize=address` produces a report that
   names everything, which is why the course builds everything twice.
3. **Grep for the copies into fixed buffers.** The check only fires on destinations whose
   size the compiler knew, which is a short list: local arrays and struct members.

## Why both flags are needed

Fortification is the compiler and the library together:

- the **header** (`<string.h>` under `_FORTIFY_SOURCE`) redirects `strcat` to
  `__strcat_chk(dst, src, __builtin_object_size(dst, 0))`;
- the **compiler** computes that size, and `__builtin_object_size` returns "unknown" (−1)
  unless optimisation has told it what the pointer points at;
- the **library** compares at run time and calls `__chk_fail` if the copy would not fit.

Remove the macro and the header does not redirect; remove `-O2` and the size is unknown, so
the check is compiled away. Both experiments in the README's last section print the greeting
and exit 0.

This is also why the course's normal build passes `-U_FORTIFY_SOURCE` (SPEC section 9.1):
distributions differ in whether they define it by default, and the course wants every CI leg
running the same code. This exercise turns it back on through `EXTRA_CFLAGS`, which
`common.mk` places *after* the `-U`, so the macro wins.

## Alternatives

The four tools of this unit, as a table to keep:

| Tool | When it works | What it names | Cost |
|---|---|---|---|
| compiler warning | both sizes constant at compile time | file, line, flag | none |
| `_FORTIFY_SOURCE` | destination size known, any source | nothing | a comparison per call |
| AddressSanitizer | any access leaving an object | everything | 2× time, 3× memory |
| a check in the program | whatever you thought to check | whatever you print | whatever you wrote |

The fourth row is the one this unit keeps coming back to. e05's intra-struct overflow is
invisible to the first three and obvious to the fourth.

In production, fortification is close to free and worth having on: Debian, Ubuntu, Fedora and
Arch all build their packages with `-D_FORTIFY_SOURCE=2` or `=3` (the newer level uses a
smarter object-size computation and catches more). It will not find your bugs for you, and it
turns a few of them from an exploit into a crash.

## The usual mistake

Answering `exit.status` with 6. Signal 6 is `SIGABRT`; the *status* a shell reports for a
process killed by signal N is 128 + N, so 134. The course's `lib/course.h` lists the
convention, and `make` printed `Error 134`.

Second: answering `stopped_by` with `compiler`. The compiler's contribution is the size
argument; the comparison, the message and the `abort()` are in libc. The distinction matters
because it tells you where to look for documentation — `man 7 feature_test_macros` and
glibc's manual, not gcc's.

Third: expecting the message to name something. It never does. A reader who has not seen it
before tends to assume the output is truncated or that a verbose mode exists; there is
neither.
