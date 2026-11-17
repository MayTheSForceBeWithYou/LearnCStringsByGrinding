# s02-e04-memset-args: answer

## The defect

`slot.c`, line 9, in `fill_bytes`:

```c
    memset(p, size, value);
```

`memset`'s parameters are `(void *s, int c, size_t n)`: pointer, **byte value**, **count**.
The call passes the count where the value belongs and the value where the count belongs. The
fix is one line:

```c
    memset(p, value, size);
```

## The mechanism

`slot_clear` calls `fill_bytes(s, sizeof *s, 0)`, so `size` is 24 (an `int` and a 12-byte
array, padded to 24 on both course platforms) and `value` is 0. The swapped call is therefore
`memset(p, 24, 0)`: **fill zero bytes with the byte 24**. `memset` writes nothing, returns
`p`, and the slot keeps every byte it had.

So after `slot_clear`, storage that was scribbled with `0x5A` still holds `0x5A` everywhere.
The three checks fail in the three ways that follow: `all_zero` finds a non-zero byte, `id`
is `0x5A5A5A5A` rather than 0, and `name` is the twelve `Z` bytes the harness printed as
`ZZZZZZZZZZZZ\0` — the terminator being the thirteenth byte, outside the field.

The call is not an error in any sense C recognises. Both parameters are integers, `24`
converts to `int` for the value and `0` converts to `size_t` for the count, and a count of
zero is explicitly allowed. The function does exactly what it was asked.

Had the two arguments been 12 and 65 — say `fill_bytes(p, 12, 'A')` in a routine that fills a
name field with a character — the swap would have written **65 bytes** of the value 12 into a
12-byte field: a 53-byte overflow, reported instantly by AddressSanitizer as a
`WRITE of size 1 ... 0 bytes after` whatever object the field lives in. The transposition is
only silent because the count that ended up in the value slot was zero. The same mistake with
any other pair of numbers is either a buffer overflow or a buffer full of the wrong byte.

## How the tool showed it

No sanitizer reports anything, because nothing is written: there is no out-of-bounds access,
no undefined behaviour, no leak. The program is simply wrong.

The compilers do have a diagnostic for this, `-Wmemset-transposed-args`, and both of them fire
when the count argument is a literal zero that they can see. From the lesson's
`lesson-demo/capture-warn.txt`, gcc on a version of the call written in plain sight:

```
warning: ‘memset’ used with constant zero length parameter; this could be due to transposed
parameters [-Wmemset-transposed-args]
```

and clang, from `capture-warn-clang.txt`, on the same line:

```
warning: 'size' argument to memset is '0'; did you mean to transpose the last two arguments?
[-Wmemset-transposed-args]
```

Neither says anything about `slot.c`, because the call there is inside `fill_bytes`, where
both arguments are parameters. The diagnostic depends on seeing a constant; a wrapper
function — one line, written for perfectly good reasons — removes the constant and with it the
warning.

What caught it was the harness: `all_zero` checks every byte of the slot, including the
padding, and `check_bytes` on the name field shows the leftover bytes rather than just the
string. A test that only compared `s->id == 0` and `strcmp(s->name, "") == 0` would have
failed too, here; a test that only checked the *members* after a `memset` of a freshly
declared slot might not have.

## The rule you take away

`memset` takes the byte before the count — and when a call's arguments arrive as parameters,
no compiler can check the order for you.
