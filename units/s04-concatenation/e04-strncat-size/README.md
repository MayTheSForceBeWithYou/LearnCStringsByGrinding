# s04-e04-strncat-size `[BUG]`

## Goal

A label builder appends three pieces into a fixed buffer and overflows it the moment the
pieces do not fit. The call that does it looks like the safe one: it is `strncat`, with a size
argument. Find the defect, write the correct third argument, and explain the formula.
Afterwards you know what the `n` of `strncat` bounds — and you will notice that both course
compilers have a diagnostic for this exact mistake, which did not fire here.

## Background

Two functions in `<string.h>` take an `n`, and the `n` means different things:

| Call | What `n` bounds | Bytes written |
|---|---|---|
| `strncpy(dst, src, n)` | the **destination**: exactly `n` bytes are written | exactly `n` |
| `strncat(dst, src, n)` | the **source**: at most `n` characters are taken from it | up to `n + 1` |

So `strncat(dst, src, sizeof dst)` says "append up to `sizeof dst` characters to whatever is
already in `dst`", which can write `sizeof dst + 1` bytes *past the existing contents*. It is
not a bound on the destination in any sense, and it is a buffer overflow as soon as the source
is long.

The correct third argument is the room left for characters:

```c
strncat(dst, src, cap - strlen(dst) - 1);
```

`cap` is the buffer's total size, `strlen(dst)` is what is already there, and the `- 1` is the
terminator `strncat` always writes on top of whatever it appended. The Linux `strncat(3)` page
states the requirement from the other side: the buffer "must be at least `strlen(dst) +
strnlen(src, ssize) + 1`".

Both course compilers have a warning for the `sizeof dst` form — gcc's
`-Wstringop-truncation`/`-Wstringop-overflow=` and clang's `-Wstrncat-size`, which even prints
the expression you should have written. The lesson's **What the tools say** has both. Neither
fires on this file, because the size arrives through a parameter: the same hiding place as
S02 e04, and the same lesson.

Read **Foundations** and the `strncat` card in `LESSON.md` before you start.

## Symptom

`make check` fails in the plain build. From `capture-run.txt`, produced from exactly this
source:

```
FAIL main.c:20: an append of one byte too many
       got:  abcdefgh\0
       want: abcdefg\0
FAIL main.c:21: label_append writes nothing outside the buffer
FAIL main.c:20: an append far too long
```

The first line shows an 8-byte buffer holding nine bytes: `abcdefgh` and a terminator after
it. The second is the fence check — the bytes around the buffer have been overwritten, which
in the plain build is how the harness notices.

`make asan` stops at the first bad write instead. From `capture.txt`:

```
==1934973==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x756ee8ee0098 at pc ...
WRITE of size 3 at 0x756ee8ee0098 thread T0
    #0 ... in strncat (/usr/lib/libasan.so.8+0x124669)
    #1 ... in append_n .../e04-strncat-size/label.c:9
    #2 ... in label_append .../e04-strncat-size/label.c:17
    #3 ... in append_case .../e04-strncat-size/main.c:19
```

`WRITE of size 3` is the interceptor reporting the whole range `strncat` was about to write.
Frame `#0` is ASan's replacement for `strncat`; frame `#1` is the one-line helper; frame `#2`
is where the argument was chosen, and that is the defect.

## What the code is supposed to do

`label.h` is the specification and it is correct.

- `label_append(dst, cap, src)` appends `src` to the string in `dst`, which has `cap` bytes in
  all, truncating if it does not fit. `dst` is a string afterwards. It returns 0 if the whole
  of `src` was appended, -1 if anything was dropped, and it never writes `dst[cap]` or beyond.
- `label_build(dst, cap, kind, text)` builds `"<kind>: <text>"` the same way, returning -1 if
  anything was truncated.

| Call | `dst` (cap 8) afterwards | Returns |
|---|---|---|
| `label_append(dst, 8, "cd")` on `ab` | `abcd` | 0 |
| `label_append(dst, 8, "g")` on `abcdef` | `abcdefg` | 0 (an exact fit) |
| `label_append(dst, 8, "gh")` on `abcdef` | `abcdefg` | -1 |
| `label_append(dst, 8, "cdefghij")` on `ab` | `abcdefg` | -1 |
| `label_append(dst, 8, "h")` on `abcdefg` | `abcdefg` | -1 (nothing fits) |
| `label_build(dst, 11, "warn", "disk")` | `warn: disk` | 0 |
| `label_build(dst, 10, "warn", "disk")` | `warn: dis` | -1 |
| `label_build(dst, 5, "warn", "disk")` | `warn` | -1 |
| `label_build(dst, 1, "warn", "disk")` | `` | -1 |

## What to hand in

1. **The fix**, in `label.c`. There is exactly one defect, in one function, and nothing else
   in the exercise is wrong. Keep the helper, keep both signatures, and do not change
   `main.c` or `label.h`.
2. **`ANSWER.md`**, with its four headings filled in:
   - *The defect*: the file and line, the line quoted, and what `strncat`'s third parameter
     means according to its card and its manual page.
   - *The mechanism*: for `label_append(dst, 8, "gh")` with `dst` holding `"abcdef"` — how
     many bytes does `strncat` write, where does the last one go, how far past the buffer is
     it? Then the formula for the correct third argument, with every term explained,
     including the `- 1`.
   - *How the tool showed it*: the report's error kind, access kind and size; which frame is
     the interceptor and which is the first line of course code; and what the plain build's
     two failing checks each added. Why does neither compiler warn about this file when both
     have a diagnostic for the mistake?
   - *The rule you take away*: one sentence.
3. Make sure the fix handles `cap == 0` without computing `cap - strlen(dst) - 1`, which would
   wrap.

## Constraints

- `label_append` must still return 0 for an exact fit and -1 for any truncation.
- `dst` must be a string after every call, including the ones that truncate.
- No allocation, and no second buffer.
- Do not replace `strncat` with `snprintf` or a hand-written loop: the exercise is the
  argument, not the function.

## Verify

```bash
make check
```

Done means `s04-e04-strncat-size: all checks passed` in the plain stage and again in the ASan
stage, with no report, and `== check: OK`.

Afterwards, in a scratch file outside this directory, write the same call with the sizes in
plain sight — `char buf[8] = "ab"; strncat(buf, src, sizeof buf);` — and compile it with
`-Wall -Wextra -O2`. Both compilers object, and clang prints the expression you should have
used. Then move the call behind a helper that takes the size as a parameter and watch the
diagnostic disappear.

<details><summary>Hint</summary>

Three terms: the buffer's size, what is already in it, and the byte `strncat` always writes
afterwards.

</details>
