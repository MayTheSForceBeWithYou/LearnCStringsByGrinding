# s02-e05-struct-compare: answer

## The defect

`point.c`, line 17, in `point_eq`:

```c
    return memcmp(a, b, sizeof *a) == 0;
```

`point_eq` is the function at fault. The other `mem` call in the file, `memcpy` in
`point_copy`, is correct: copying all the bytes of an object — padding included — is exactly
what a copy should do, and it is what struct assignment does too. Comparing all the bytes is
not what a comparison should do, because not all the bytes are the value.

## The mechanism

On both course platforms a `Point` is eight bytes:

```
offset:   0    1    2    3    4    5    6    7
          kind |--- padding ---| |---- value ----|
```

`char kind` takes byte 0. `int value` has to start at a multiple of 4, so the compiler
inserts three bytes of padding at offsets 1, 2 and 3, and `value` occupies 4 to 7.
`sizeof(Point)` is 8, while the members together are 5 bytes.

The two points in the first check are built by `dirty_point`, which fills the storage with a
byte pattern and then assigns the members. So:

| | byte 0 | bytes 1-3 | bytes 4-7 |
|---|---|---|---|
| `a`, scribbled with 0x00 | `'p'` | 00 00 00 | 42 |
| `b`, scribbled with 0x5A | `'p'` | 5A 5A 5A | 42 |

Both have `kind == 'p'` and `value == 42`; the check immediately above confirms it. `memcmp`
compares all eight bytes, finds a difference at offset 1, and returns non-zero, so `point_eq`
says they are different.

The padding is allowed to differ because the standard says so. C17 6.7.2.1p15 permits unnamed
padding within a structure, and 6.2.6.1p6 says that when a value is stored in a member of a
structure, the bytes of the object that correspond to any other member or to padding take
**unspecified** values. Assigning `p.kind` and `p.value` says nothing at all about bytes 1 to
3; they keep whatever the storage held, and no two pieces of storage have to agree.

## How the tool showed it

It did not, and no tool could.

- **AddressSanitizer** reports accesses outside an object. All eight bytes are inside the
  `Point`, so there is nothing to report.
- **UndefinedBehaviorSanitizer** reports undefined behaviour. Reading an object's padding
  through `memcmp` is not undefined: the bytes are part of an object the program owns, and
  `unsigned char` access to them is explicitly allowed. Their *values* are unspecified, which
  is a weaker statement than undefined and carries no diagnostic.
- **The compiler** sees a well-typed call to a standard function with the right arguments.
  There is nothing to warn about. (MemorySanitizer, which is clang-only and not part of this
  course, does track uninitialised bytes and would complain about a `memcmp` that reads
  padding which was never written — but here the padding *was* written, by the scribble, so
  even MSan would be satisfied.)

What caught it were two checks: `main.c:37`, which asserts that two points with equal members
are equal, and `main.c:73`, the table search. The search is the shape this defect takes in a
real program: records are built in several places — one from a file, one from a form, one from
a default — and a lookup that compares them byte-wise finds the key only when its padding
happens to match. It works in testing, where everything is built the same way, and fails in
production, where it is not.

## The rule you take away

Compare structs member by member: `memcmp` compares the padding too, and the padding is not
part of the value.
