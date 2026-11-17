# s02-e03-overlap-report: notes

## Why each answer is what it is

| Key | Answer | Where it comes from |
|---|---|---|
| `error.kind` | `memcpy-param-overlap` | the `ERROR:` line |
| `range.size` | `27` | `0x...4b - 0x...30` = 0x1b = 27, and the second range is the same size |
| `gap.bytes` | `4` | `0x...34 - 0x...30` |
| `overlap.bytes` | `23` | 27 − 4: the ranges are the same length, offset by 4 |
| `fault.file` | `text.c` | frame `#1` |
| `fault.line` | `16` | the same frame |
| `fix.function` | `memmove` | the function whose contract allows overlap |
| `forward_loop_ok` | `yes` | the destination is *below* the source here |
| `move.bytes` | `27` | `text_move_count(30, 0, 4)` = 30 − 0 − 4 + 1: the tail plus the terminator |

`range.size` and `move.bytes` are the same number from two directions — the report's
arithmetic and the source's — which is the cross-check the README asks for.

## Why `forward_loop_ok` is yes, and why that is not a defence

The destination is `buf + 0` and the source is `buf + 4`, so the destination is lower. A
forward loop writes `d[0]` after reading `s[0]`, which is `buf[4]`; by the time it writes
`buf[4]` (as `d[4]`) it has already read it. Every source byte is read before the write that
would clobber it, so a forward loop gives the right answer — which is exactly why the plain
run prints the right string.

None of that makes the call correct. `restrict` is a promise, and the program broke it; the
implementation is then entitled to copy in any order it likes, and glibc's x86-64 `memcpy`
copies large blocks from both ends inwards. The reason this particular call survives is that
27 bytes is small enough to take a path that happens to be safe for this direction. Change the
size, the compiler, the CPU's feature bits or the libc version and the same source produces
different bytes. "It works" is not a property of the program here; it is a property of this
afternoon.

The direction argument is still worth having, because it is what `memmove` does internally
(e01), and because it tells you which in-place edits are dangerous: moving text *down* (a
deletion) is the safe direction for a forward copy, and moving text *up* (an insertion) is the
one that visibly corrupts.

## Reading order that gets there fastest

1. The `ERROR:` line names the kind. For `memcpy-param-overlap` there is no out-of-bounds
   byte, so there is no "is located" line and no allocation stack: everything is in that line.
2. Two ranges, `[start, end)`. The **first is the destination** — the order of `memcpy`'s
   parameters.
3. Length of each: `end - start`, in hex.
4. Distance between them: the difference of the two starts.
5. Overlap: for equal-length ranges, length − distance (and 0 if that is negative).
6. The first frame in your own code is the call. The fix is on that line.

## Alternatives

The one-word fix is `memmove`. Three other ways to write the same deletion, in case the
surrounding code makes one of them natural:

```c
/* a loop, in the safe direction for a deletion */
for (size_t i = 0; i < move; i++)
    buf[at + i] = buf[at + n + i];

/* copy via the end pointer, which is what a gap buffer does */
memmove(buf + at, buf + at + n, len - at - n + 1);

/* build the result in a second buffer: no overlap at all, at the cost of the buffer */
memcpy(out, buf, at);
memcpy(out + at, buf + at + n, len - at - n + 1);
```

The second is the fix. The first is correct here and will be wrong the day someone copies it
into an insertion. The third is what a program does when it can afford the second buffer, and
it is the only one of the three that is obviously correct at a glance — which is an argument
for it in code that does not need the speed.

## The usual mistake

Reading the two ranges as "the overlapping part" rather than as the two whole arguments. They
are the destination and the source of the same call, each `n` bytes long; the overlap is what
they have in common, which takes two subtractions to find.

Second: answering `21` for `range.size` by subtracting the hex digits as if they were decimal
(`0x4b - 0x30` read as 51 − 30). The addresses are hexadecimal; the arithmetic has to be too.

Third: concluding from the plain run that the code is fine. It printed the right string, and
the call is still undefined behaviour. The entire value of a sanitizer is that it reports what
the hardware forgave.

Fourth: "fixing" it by making the copy a loop. The loop is correct for a deletion and wrong
for an insertion, and the next person to edit the function has no way to know which case they
are in. `memmove` is correct for both and says so in its name.

## How libc does it

ASan's `memcpy` interceptor compares the two ranges before forwarding to the real
implementation; that check is what produced this report, and it exists precisely because the
real implementation cannot be relied on to be forgiving. glibc's `memcpy` on x86-64 copies
with vector registers and, above a size threshold, from both ends towards the middle — a
pattern that is correct for disjoint regions and produces scrambled output for overlapping
ones in either direction.

`memmove` in the same library shares that machinery and adds the comparison of the two
pointers. For large copies it moves whole blocks from the appropriate end; for small ones it
may load everything into registers first and store afterwards, which makes the overlap
irrelevant. None of that is visible in the contract, and none of it needs to be: the standard
describes the result, and the implementation is free to reach it however the processor is
fastest.
