# s03-e05-unterminated-name: answer

## The defect

`roster.c`, in `set_field`:

```c
strncpy(dst, src, cap);
```

called from `member_new` as `set_field(m->name, sizeof m->name, name)`, so `cap` is 12, the
full size of the field.

## The mechanism

`strncpy` writes exactly 12 bytes. For `"Maximilienne"` (12 characters) those are the 12
characters `M a x i m i l i e n n e`. The source's terminator would be the 13th byte and is
never copied; there is no padding because there is nothing left to pad. The `name` field
therefore contains no NUL, and it is not a string.

The field occupies bytes 4 to 15 of a 16-byte heap block. `strlen(m->name)` at `main.c:19`
reads bytes 4 to 15 of the block, finds no NUL, and reads byte 16, which is outside the
block. `member_name_was_cut` and `member_label` would do the same.

## How the tool showed it

- `heap-buffer-overflow`: an access outside a heap block.
- `READ`: the program read; it did not write. For a string that means "no terminator".
- `of size 13`: twelve bytes of the field and the first byte after the block.
- Frame `#0` (in `libasan.so`): ASan's replacement for `strlen`, where the access was caught.
- Frame `#1`, `main.c:19`: the call to `strlen`. Not the defect.
- `is located 0 bytes after 16-byte region`: the byte immediately after a 16-byte block.
  16 is `sizeof(Member)`: 4 for `id`, 12 for `name`.
- `allocated by ... member_new roster.c:16`: which block. Its last field is the one that was
  filled without a terminator.

The plain build does not crash: `malloc(16)` returns a chunk with spare bytes after the 16,
and `strlen` stops at whatever zero byte happens to be there. The checks fail only because
the length is wrong.

## The rule you take away

`strncpy(dst, src, sizeof dst)` does not produce a string; pass `sizeof dst - 1` and set the
last byte, or use `snprintf`, and look for the writer when a report names a reader.
