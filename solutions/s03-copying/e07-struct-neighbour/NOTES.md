# s03-e07-struct-neighbour: notes

## Why it is correct

The defect was `strcpy(a->user, name)` with no length check. The fix measures first:

```c
size_t len = strlen(name);
if (len >= sizeof a->user)
    return -1;
memcpy(a->user, name, len + 1);
```

`len >= sizeof a->user` is "does not fit": a 16-byte field holds at most 15 characters. The
rejection happens before any byte of the account is written, so "changes nothing" holds
trivially. On success, `len + 1` bytes are copied, terminator included, all inside `user`.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| 15 characters | `15 >= 16` is false; 16 bytes copied; `role` untouched |
| 16 characters | `16 >= 16` is true; rejected. With the bug, the terminator landed in `role[0]` and the role became `""` |
| 21 characters ending in `admin` | rejected. With the bug, `role` became `"admin"` |
| empty name | 1 byte copied |

## The mechanism, in bytes

`Account` after `account_init` and `account_set_user(&a, "fifteen_chars_x")`:

```
user (16 bytes)                                  | role (8 bytes)
f i f t e e n _ c h a r s _ x \0                 | g u e s t \0 \0 \0
```

After the buggy `strcpy(a->user, "mallory_________admin")`, which writes 22 bytes:

```
m a l l o r y _ _ _ _ _ _ _ _ _                  | a d m i n \0 \0 \0
```

Characters 1 to 16 fill `user`, leaving it with no terminator of its own; characters 17 to
21 and the terminator land in `role`. `strcmp(a->role, "admin")` is now 0.

## Why no tool reported it

ASan poisons the memory *around* each object and checks that every access stays out of the
poisoned zones. `Account a` in `main` is one 24-byte stack object. All 22 bytes written by
the `strcpy` lie inside it, so every one of them is addressable and nothing is reported.
ASan's `strcpy` interceptor checks the range `[dst, dst + len + 1)` against the shadow
memory; it has no idea that `dst` was meant to be a 16-byte member.

UBSan's bounds check covers array indexing written in the source (`a->user[i]`), not what a
library function does with a pointer. The compiler's warning needs the source length at
compile time. Only the harness's check on `role` saw anything.

## Alternatives

`snprintf(tmp, sizeof tmp, "%s", name)` into a local buffer and a check of its return value
also works, at the price of a second copy. A truncating copy is wrong here whatever the
tool: a login name cut to 15 characters is a different login name.

Reordering the struct (`role` first) hides this symptom and keeps the overflow.

## The usual mistake

Checking `len > sizeof a->user` (off by one: accepts 16 characters and writes the terminator
into `role[0]`), and believing that a clean sanitizer run means there is no overflow.

## How libc does it

With `_FORTIFY_SOURCE` and optimisation, glibc's headers replace `strcpy` with
`__strcpy_chk(dst, src, __builtin_object_size(dst, 1))`. At level 2 the "1" asks for the size
of the *member*, so the fortified build aborts on this overflow with `*** buffer overflow
detected ***` where ASan stays silent. S05 looks at that report.
