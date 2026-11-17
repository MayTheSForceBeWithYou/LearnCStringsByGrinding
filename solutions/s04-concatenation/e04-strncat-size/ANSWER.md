# s04-e04-strncat-size: answer

## The defect

`label.c`, line 17, in `label_append`:

```c
    append_n(dst, src, cap);
```

`append_n` passes its third argument straight to `strncat`, whose third parameter is **how
many characters may be taken from the source**, not how large the destination is. The Linux
`strncat(3)` page: "This function appends at most ssize non-null bytes from the array pointed
to by src, followed by a null character, to the end of the string pointed to by dst," and it
states the buffer requirement as `strlen(dst) + strnlen(src, ssize) + 1`.

The fix is the room left for characters:

```c
    append_n(dst, src, cap - have - 1);
```

with `cap == 0` returned early, so the subtraction cannot wrap.

## The mechanism

For `label_append(dst, 8, "gh")` with `dst` holding `"abcdef"`:

- `strlen(dst)` is 6, so the terminator is at `dst[6]` and `dst[7]` is the last byte of the
  buffer.
- `strncat(dst, "gh", 8)` is allowed to append up to 8 characters; the source has 2, so it
  appends both — at `dst[6]` and `dst[7]` — and then writes its terminator at `dst[8]`.
- `dst[8]` is one byte past an 8-byte buffer. For the longer source `"cdefghij"` with `dst`
  holding `"ab"`, it appends all 8 characters at `dst[2]`..`dst[9]` and terminates at
  `dst[10]`: three bytes past the end.

The correct third argument is

```
cap - have - 1
```

- `cap` is the buffer's total size in bytes;
- `have` is `strlen(dst)`, the characters already there, so `cap - have` is the bytes left
  starting at the terminator;
- the `- 1` is the byte `strncat` always writes for its own terminator, on top of whatever it
  appended.

So with `cap` 8 and `have` 6 the correct bound is 1: one character may be appended, at
`dst[6]`, with the terminator landing on `dst[7]`, the buffer's last byte.

## How the tool showed it

AddressSanitizer reports `heap-buffer-overflow` with `WRITE of size 3`: the size is the whole
range the intercepted `strncat` was about to write, not one byte, because ASan's interceptor
checks the full write before performing it. Frame `#0` is inside `libasan` — ASan replaces
`strncat` with a checking version — frame `#1` is the one-line helper `append_n`, and frame
`#2` is `label_append`, which is where the argument was chosen and where the fix goes.

The plain build does not crash, because the overflow lands in the fence bytes the harness puts
around every buffer. It produces two failing checks per case, and the pair is the useful part:
`check_str` shows what the buffer holds (`abcdefgh\0`, nine bytes in an eight-byte buffer),
and `check_guard` says that the bytes *outside* the buffer were touched — which `check_str`
alone could never tell you.

Neither compiler says anything about this file, although both have a diagnostic for exactly
this mistake: gcc's `-Wstringop-truncation` and `-Wstringop-overflow=`, and clang's
`-Wstrncat-size`, which even suggests `sizeof(buf) - strlen(buf) - 1`. Both need to see the
size as a constant at the call. Here it arrives as a parameter of `append_n`, so there is
nothing for them to analyse — the same hiding place as S02 e04's transposed `memset`.

## The rule you take away

`strncat`'s `n` bounds the source, not the destination: the only correct value is the room
left for characters, `cap - strlen(dst) - 1`.
