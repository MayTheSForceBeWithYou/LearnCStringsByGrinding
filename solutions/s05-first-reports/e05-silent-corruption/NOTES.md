# s05-e05-silent-corruption: notes

## Why each answer is what it is

The struct is sixteen bytes with no padding: two `char[8]` members, each needing only byte
alignment.

```
offset:  0  1  2  3  4  5  6  7 | 8  9 10 11 12 13 14 15
         <------- user -------> | <------- role ------->
```

| Key | Answer | Reason |
|---|---|---|
| `struct.size` | `16` | 8 + 8, no padding |
| `user.offset` | `0` | the first member |
| `role.offset` | `8` | immediately after `user` |
| `fits.bytes` | `ada\0\0\0\0\0guest\0\0\0` | `account_init` zeroes all 16 and writes `guest` at 8; `"ada"` writes 4 bytes at 0 |
| `fits.role` | `guest` | untouched |
| `exact.role` | `guest` | `"abcdefg"` writes 8 bytes: 7 characters and a terminator at offset 7 |
| `over1.bytes` | `abcdefgh\0uest\0\0\0` | 9 bytes written; the terminator lands on `role[0]` |
| `over1.role_len` | `0` | `role[0]` is a zero byte, so the role is the empty string |
| `attack.user` | `mallory_admin` | reading `user` stops at the first zero byte, now at offset 13 |
| `attack.role` | `admin` | bytes 8..12 are `admin`, terminator at 13 |
| `attack.is_admin` | `1` | `strcmp(a->role, "admin")` is 0 |

## The three cases, byte by byte

```
exact    "abcdefg"       a b c d e f g \0 | g u e s t \0 \0 \0    role still "guest"
over1    "abcdefgh"      a b c d e f g  h | \0 u e s t \0 \0 \0   role is now ""
attack   "mallory_admin" m a l l o r y  _ | a d m i n \0 \0 \0    role is now "admin"
```

The exact case is the boundary: seven characters and a terminator fill `user` precisely. One
more character and the terminator — the byte nobody counts — lands in the neighbour.

The attack case shows what the extra bytes are *for*: eight characters to fill `user`, then
the five of `admin`, then the terminator `strcpy` supplies free. Nothing in the program ever
assigns to `role`.

## Why no tool says anything

| Tool | Why it is silent |
|---|---|
| AddressSanitizer | every byte written is inside the 16-byte object; no redzone is touched |
| UndefinedBehaviorSanitizer | nothing here is undefined in the ways it checks |
| the compiler | the destination arrives as a parameter, so neither size is a constant |
| Valgrind | same as ASan: the write is within an allocated object |

The tool that *can* see it is a check on the neighbour's value, which is what the eleven
answers are. In a real program that is a test asserting a record's invariants after an
operation.

clang can be told to insert redzones inside structs
(`-fsanitize=address -mllvm -asan-detect-intra-object-overflow`), and it is off by default
because it changes layout and breaks anything that reads a struct as bytes.

## Alternatives

Every fix from S03 applies to the unbounded copy:

```c
if (strlen(name) >= sizeof a->user) return -1;    /* refuse (S03 e06)    */
snprintf(a->user, sizeof a->user, "%s", name);     /* truncate and report */
strncpy(a->user, name, sizeof a->user);            /* fill, no terminator */
```

The design fix is different: two fields that must not run into each other should not be
adjacent in a struct filled from outside. Real formats put a length before each field, or
keep the data and the metadata apart — which is what a `(pointer, length)` view does, and
what S19 is about.

## The usual mistake

Forgetting the terminator in the over-by-one case. Eight characters "fit" in an eight-byte
field only if you do not count the ninth byte `strcpy` writes.

Second: expecting `attack.user` to read as `mallory_`. Reading a field is not bounded by the
field: `%s` and `strcmp` stop at the first zero byte, and after the overflow the first one is
at offset 13.

Third: expecting ASan to catch it. The rule is precise — ASan reports accesses that leave an
**object** — and a struct is one object.
