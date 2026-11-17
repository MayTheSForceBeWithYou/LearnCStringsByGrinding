# s02-e04-memset-args: notes

## Why it is correct

The fix is the parameter order:

```c
memset(p, value, size);
```

`memset`'s contract is "write `n` copies of `(unsigned char)c` at `s`", with the value second
and the count third. With the arguments in that order, `slot_clear` asks for 24 bytes of the
value 0, which is what "clear this slot" means.

Clearing with `memset` over `sizeof *s` — rather than assigning the members — is deliberate
and is what the header promises. A struct has bytes that are not members: on both course
platforms a `Slot` is 24 bytes, of which 4 are the `id`, 12 are the `name`, and some are
padding the compiler inserted for alignment. Assigning `s->id = 0; s->name[0] = '\0';` leaves
every other byte as it was, and the contract says "every byte", because the record is reused
and may later be written to a file or hashed whole.

## Edge cases and which line handles each

| Case | Handled by |
|---|---|
| padding between members | `sizeof *s` covers the whole object, not just the members |
| bytes after the name's terminator | the same: the field is cleared, not just terminated |
| a table of slots | `slot_clear_all`'s loop, one `slot_clear` per element |
| reusing a slot that held a longer name | the clear happens before the next `slot_set` |
| a name that does not fit | `slot_set`'s `len + 1 > SLOT_NAME` test, which was never the defect |

## Alternatives

```c
Slot empty = {0};        /* every member zero-initialised */
*s = empty;
```

This is the idiomatic C way to clear a struct, and for most purposes it is better than
`memset`: it is typed, it cannot get an argument order wrong, and a compiler will usually
produce the same code. One caveat worth knowing: `= {0}` guarantees that every *member* is
zero, and says nothing about the padding bytes (C17 6.7.9p21 covers members; padding is
unspecified). In practice compilers zero the whole object, and if your program depends on the
padding — because it hashes the struct, writes it to a file, or compares it with `memcmp` —
`memset` over `sizeof` is the version that says so.

That dependency is itself a smell, and it is e05 of this unit: comparing structs by their
bytes is the defect that padding causes. A program that clears whole objects and compares
members never has to think about either.

For an array, `memset(table, 0, n * sizeof *table)` does the lot in one call; drill r02 is
that line four ways, including the `sizeof *p` habit that keeps the size and the type from
drifting apart.

## The usual mistake

The transposition itself, which this exercise is. Two things make it rare in practice:
writing the call with its arguments in plain sight, where the compiler's
`-Wmemset-transposed-args` can see them, and preferring `= {0}` for structs so the question
does not arise.

The second mistake is "fixing" it by assigning the members:

```c
s->id = 0;
s->name[0] = '\0';
```

Both checks on `id` and on the name pass, and `all_zero` still fails, because the padding and
the bytes after the terminator keep the old record. The header says "every byte" for a
reason, and a test that checks it is the only thing standing between this program and a
record file with other people's names in its unused space.

The third is removing `fill_bytes` and calling `memset` directly from `slot_clear`. The
program becomes correct and the lesson is lost: the wrapper is not the defect, it is the
reason no tool caught the defect, and real code is full of wrappers.

## How libc does it

`memset`'s odd-looking `int` value parameter is a fossil of pre-prototype C, where a `char`
argument was promoted to `int` before the call anyway; the standard keeps the signature and
specifies the conversion to `unsigned char` instead (C17 7.24.6.1). That is why
`memset(p, -1, n)` fills with `0xFF` and `memset(p, 0x141, n)` fills with `'A'`.

glibc's implementation broadcasts the byte into a vector register and stores 32 or 64 bytes at
a time, with special paths for small sizes and, for very large ones, non-temporal stores that
bypass the cache. For a 24-byte struct it is a couple of stores.

One thing no `memset` can promise: that the bytes stay written. A compiler is allowed to
remove a `memset` whose result is never read — which is why clearing a password buffer before
`free` needs `memset_explicit` (C23) or an equivalent, and why `explicit_bzero` exists on
Linux and BSD. It is also why this course's `FORBID` lists mention it: SPEC section 10 keeps
those out of the portable set, and the problem is worth knowing about long before you need it.
