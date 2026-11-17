# s01-e02-fixed-fields `[WRITE]` (S)

## Goal

Read fixed-width, NUL-padded record fields correctly: measure one, compare two, and copy one
out as a string. The field may be full, which means it has no terminator, so every function
here is bounded and none of them may call `strlen`. Afterwards you can work with the record
formats that real systems use — database rows, protocol headers, tar archives, card images —
without turning each field into an over-read.

## Background

A fixed-width field is `width` bytes holding text, padded with NUL bytes when the text is
shorter. It is the oldest record layout there is and it is still everywhere, because it makes
a record a fixed size: the fifth field of the ninth row is at a known offset, with no parsing.
Early Unix directory entries were 14-byte name fields; `strncpy` was written to fill them
(S03), and the padding is what makes two equal names compare equal byte for byte.

The catch is the full field. If the text is exactly `width` characters long there is no room
for a terminator, and the field is a **character sequence**, not a string. The library's
unbounded functions — `strlen`, `strcpy`, `strcmp`, `%s` — have no way to stop at the field's
end, so handing one of them a full field reads into whatever follows it. That is not a rare
corner: the full field is the normal case for a format whose widths were chosen to fit the
data.

So a field reader uses the bounded functions, and the bound is the field's width, which the
format tells you. `strnlen(field, width)` answers "how long is the text" and, at the same
time, "is this field full": a result equal to `width` means there was no terminator. From
there, `memcpy` moves the exact number of bytes you measured and you write the terminator
yourself.

The second thing this exercise teaches is that **the padding is not part of the value**. Two
fields are equal when their text is equal; what follows the first NUL is leftover, and a
reader that compares all `width` bytes will report two records with the same name as
different. Writers are supposed to zero the padding, and plenty of real writers do not.

Read **Foundations: a length is not a size** and the `strnlen` card in `LESSON.md` first.

## The API

`field.h` declares:

```c
size_t field_len(const char *field, size_t width);
int field_eq(const char *a, const char *b, size_t width);
size_t field_to_cstr(char *dst, size_t cap, const char *field, size_t width);
```

In all three, `field` (and `a`, `b`) is borrowed, is never modified, and has **exactly**
`width` readable bytes. Reading `field[width]` is a defect, not a convenience: the harness
puts every field in a buffer of exactly its width.

**`field_len(field, width)`** returns the number of bytes before the first NUL, or `width` if
there is no NUL among the `width` bytes. It reads at most `width` bytes and stops at the first
NUL. `width == 0` gives 0 and reads nothing. A result equal to `width` means the field is
full and holds no terminator.

**`field_eq(a, b, width)`** returns 1 if the two fields hold the same text, else 0. The
padding is not part of the text, so the test is "equal lengths and equal text bytes"; bytes
after the first NUL of either field are not compared. Two empty fields are equal.
`width == 0` gives 1 and reads nothing.

**`field_to_cstr(dst, cap, field, width)`** copies the field's text into `dst` as a string.
`dst` is the caller's buffer of `cap` bytes, which may be 0, and then nothing is written. At
most `cap - 1` characters are copied, and `dst` is always terminated when `cap > 0`. It
returns `field_len(field, width)`, the length it tried to produce — so a return value `>= cap`
means the text was truncated, and `ret + 1` is the capacity that would have been enough. This
is the `snprintf` contract, and you will meet it again in S03, S04, S11 and S14.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `field.c`, below the marked line.

## Behaviour

Fields are 8 bytes unless stated; `\0` is a NUL byte, and destination buffers show `#` for a
byte nothing wrote.

| Call | The field's bytes | Result |
|---|---|---|
| `field_len(f, 8)` | `ada\0\0\0\0\0` | `3` |
| `field_len(f, 8)` | `maximili` | `8`: full, no terminator |
| `field_len(f, 8)` | `\0\0\0\0\0\0\0\0` | `0` |
| `field_len(f, 8)` | `ada\0xxxx` | `3`: it stops at the first NUL |
| `field_len(f, 4)` | `A123` | `4` |
| `field_len(f, 0)` | (no bytes) | `0`, and nothing is read |
| `field_eq(a, b, 8)` | `ada\0\0\0\0\0` and `ada\0xxxx` | `1`: same text, different padding |
| `field_eq(a, b, 8)` | `ada\0\0\0\0\0` and `grace\0\0\0` | `0` |
| `field_eq(a, b, 8)` | `maximili` and `maximili` | `1`: two full fields |
| `field_eq(a, b, 2)` | `ab` and `a\0` | `0`: lengths 2 and 1 |
| `field_to_cstr(dst, 8, f, 8)` | `ada\0\0\0\0\0` | returns `3`, `dst` is `ada\0####` |
| `field_to_cstr(dst, 4, f, 8)` | `ada\0\0\0\0\0` | returns `3`, `dst` is `ada\0` (exact fit) |
| `field_to_cstr(dst, 3, f, 8)` | `ada\0\0\0\0\0` | returns `3`, `dst` is `ad\0`: truncated, `3 >= 3` |
| `field_to_cstr(dst, 9, f, 8)` | `maximili` | returns `8`, `dst` is `maximili\0` |
| `field_to_cstr(dst, 5, f, 8)` | `maximili` | returns `8`, `dst` is `maxi\0` |
| `field_to_cstr(dst, 0, f, 8)` | anything | returns the length, writes nothing |

The pattern in the last six rows: the return value never depends on `cap`. It is the length of
the text, every time, so the caller can tell truncation from success with one comparison and
can ask for the size it needs by passing `cap == 0`.

## Constraints

- **Do not call** `strlen`, `strcpy`, `stpcpy`, `strncpy`, `strcat`, `strncat`, `snprintf`,
  `strcmp`, `strncmp` or `strdup`. Every one of them either looks for a terminator that a full
  field does not have, or writes one where the caller did not ask for it. `strnlen`, `memcpy`
  and `memcmp` are the right tools and are not forbidden. The harness inspects `field.o`.
- Never read `field[i]` for `i >= width`, and never write `dst[i]` for `i >= cap`. With
  `width == 0` or `cap == 0`, do not touch that buffer at all.
- No allocation. `field_to_cstr` writes into the caller's buffer and nowhere else.
- `field_eq` must not modify either field, and `field_to_cstr` must not modify the field.

## What the harness checks

- **`field_len`** on eight fields: padded, longer-padded, full, empty, text followed by junk
  after its NUL, a 4-byte field of four characters, three bytes above 0x7F, and `width == 0`.
  Every field sits in a buffer of exactly its width, so a read of `field[width]` is a
  sanitizer report. After each call the fences are checked: these functions write nothing.
- **`field_eq`** on nine pairs: a field with itself, same text with different padding,
  different text in both orders, two empty fields, an empty against a non-empty, two equal
  full fields, a full against a shorter one, and `width == 0`. Plus the pair `ab` and `a\0` in
  a 2-byte field, where comparing all `width` bytes and comparing the text disagree.
- **`field_to_cstr`** on ten cases: every row of the table above, each with a destination of
  exactly `cap` bytes (including a zero-byte destination) and a field of exactly `width`
  bytes. Both the return value and all `cap` bytes of the destination are compared.
- **The three together**: a full 8-byte field copied into 9 bytes, then `strlen` of the copy
  compared with `field_len` of the field, and the field checked to be unchanged.

## Verify

```bash
make check
```

Success ends with `s01-e02-fixed-fields: all checks passed` and `== check: OK`, with the
forbidden-call check printing `forbid: none of [...] is called`.

The first failure most people see is comparing the whole field instead of the text:

```
FAIL main.c:67: two fields with the same text and different padding are equal
```

That case is `ada\0\0\0\0\0` against `ada\0xxxx`: both fields hold the name `ada`, and a
`memcmp` over all eight bytes says they differ. Real formats produce exactly this, because
not every writer zeroes the padding it does not use.

In the ASan stage, a `READ of size 1` with `0 bytes after 8-byte region` means something
walked off the end of a field — almost always a `strlen` where a `strnlen` was needed, or a
`memcmp` given `width` where it should have been given a measured length.

<details><summary>Hint</summary>

All three begin with the same call. Write `field_len` first and use it in the other two.

</details>
