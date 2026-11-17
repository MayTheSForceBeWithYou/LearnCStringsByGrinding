# s03-e05-unterminated-name: notes

## Why it is correct

The defect was `strncpy(dst, src, cap)` with `cap == sizeof m->name == 12`. For a name of 12
or more characters, all 12 bytes of the field receive characters and no NUL is written. The
field is the last member of a 16-byte heap block, so the first string function to read it
(`strlen` in `main.c:19`) runs off the end of the block.

The fix copies at most `cap - 1` characters and then writes the terminator into the last
byte unconditionally:

```c
strncpy(dst, src, cap - 1);
dst[cap - 1] = '\0';
```

Now the field always holds a string of at most 11 characters, which is what `roster.h`
promises. A name of 11 characters is stored whole: `strncpy` writes 11 characters, the second
line writes the 12th byte.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| name shorter than 11 | `strncpy` pads bytes up to index 10 with NULs; index 11 is set by the second line |
| name of exactly 11 | 11 characters at indices 0 to 10, terminator at 11 |
| name of 12 or more | cut to 11 characters; `member_name_was_cut` then reports 1 because the stored length is shorter |
| empty name | 11 NULs, then one more |

## Alternatives

`snprintf(dst, cap, "%s", src);` is one statement, always terminates, and does not pad. The
reference keeps `strncpy` because padding is useful in a record: two members with the same
name then have byte-identical fields, so records can be compared or hashed with `memcmp`,
and no uninitialised heap bytes are written to a file if the record is saved.

A stricter design would reject long names (see e06 and e07). Here the header says "cut", so
the fix cuts.

Not acceptable: changing `member_name_was_cut` or the harness to use `strnlen(m->name, 12)`.
That repairs one reader and leaves `member_label`'s `%s`, and every future reader, broken.

## The usual mistake

`strncpy(dst, src, sizeof dst)`. It reads as "copy, but not more than the buffer holds",
and it is exactly the call that produces a non-string when the source is as long as the
buffer. gcc names it when it can see the sizes: "specified bound 12 equals destination
size". Here the size arrived through a parameter, so the compiler said nothing.

## How the tool showed it

`heap-buffer-overflow`, `READ of size 13`: a read, so a missing terminator rather than an
oversized write; 13 bytes, twelve in the field and one beyond. Frame `#0` is ASan's `strlen`;
frame `#1` is the innocent caller. "0 bytes after 16-byte region" plus the allocation stack
identifies the `Member` block: 4 bytes of `id` and 12 of `name`, the read starting 4 bytes
in. The plain build does not crash because glibc's smallest heap chunk has 24 usable bytes;
`strlen` wanders into the spare 8 and usually finds a zero there.
