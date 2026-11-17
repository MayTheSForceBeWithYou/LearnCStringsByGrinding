# s04-e07-path-join `[FAULT]` (M)

## Goal

Join path parts into a heap string with exactly one separator between them, whatever the parts
end and start with — and survive every forced allocation failure without leaking a byte.
Afterwards you have written the measure-then-build shape that every "return an owned string"
function in this course uses, and you have seen the fault sweep hold it to account.

## Background

Joining two path parts is the function every program writes and most get slightly wrong. The
cases are not exotic: a directory that already ends in `/`, a name that begins with one, an
empty part, an absolute path that must stay absolute. Get the rule wrong and you produce
`usr//bin` (harmless, usually), `usrbin` (a different path), or `/` where you meant nothing.

The implementation question is where the size comes from. A heap string needs its length
*before* it can be allocated, and that length depends on the same separator rule as the
output. So the function has two jobs that must agree exactly: measuring and writing. The
shape that keeps them agreeing is **one pass that does either**, driven by a flag or a null
destination pointer, so the two passes are the same code rather than two copies of the same
arithmetic. When the rules change, they change once.

The other half is failure. `malloc` can fail, and this is a `[FAULT]` exercise: the harness
forces it to, one call at a time, and checks after each that the function returned NULL and
that nothing was left allocated. With one allocation that is easy; the discipline matters as
soon as there are two, and S10, S11 and S13 all have exercises where there are.

Read **Foundations** in `LESSON.md`, and the `[FAULT]` notes in this README's *What the
harness checks*, before you start.

## The API

`pathjoin.h` declares:

```c
char *path_join(const char *dir, const char *name);
char *path_join_n(const char *const *parts, size_t n);
```

Both return **one owned string**, which the caller frees with `free()`, or NULL if an
allocation failed. Nothing is allocated and left behind on a failure: these functions are
all-or-nothing.

The separator rule, in full:

- trailing `/` characters of a part are dropped, and so are the leading ones of every part
  except the first, so `"a/" + "b"`, `"a" + "/b"` and `"a//" + "//b"` all give `a/b`;
- the **first** part keeps its leading slashes, so an absolute path stays absolute:
  `"/" + "b"` is `/b` and `"/usr/" + "bin"` is `/usr/bin`;
- a part with nothing left after that trimming contributes nothing and no separator:
  `"" + "b"` is `b`, `"a" + ""` is `a`, `"" + ""` is empty, and `"" + "/"` is empty;
- a separator is written before a part's text only when something has already been written
  and it did not end in `/`.

`path_join_n` applies the same rule left to right over `n` parts; `n` may be 0, which gives
the empty string. Neither function modifies its arguments, and no argument may be NULL.

> **This starter does not build yet, on purpose.** It declares the functions you must write,
> and the harness calls them. Until you define them, the link step fails with
> `undefined reference` (Linux) or `Undefined symbols for architecture arm64` (macOS).
> There are no placeholder stubs.

Write your definitions in `pathjoin.c`, below the marked line.

## Behaviour

| Call | Result |
|---|---|
| `path_join("usr", "bin")` | `usr/bin` |
| `path_join("usr/", "bin")` | `usr/bin` |
| `path_join("usr", "/bin")` | `usr/bin` |
| `path_join("usr/", "/bin")` | `usr/bin` |
| `path_join("usr//", "//bin")` | `usr/bin` |
| `path_join("", "bin")` | `bin` |
| `path_join("usr", "")` | `usr` |
| `path_join("", "")` | `` |
| `path_join("/", "bin")` | `/bin` |
| `path_join("/", "")` | `/` |
| `path_join("", "/")` | `` |
| `path_join("caf\xC3\xA9", "men\xC3\xBC")` | `caf\xC3\xA9/men\xC3\xBC` |
| `path_join_n({"usr","local","bin"}, 3)` | `usr/local/bin` |
| `path_join_n({"usr/","/local/","/bin"}, 3)` | `usr/local/bin` |
| `path_join_n(parts, 1)` | the part alone |
| `path_join_n(parts, 0)` | `` |
| `path_join_n({"","a","","b",""}, 5)` | `a/b` |

The three rows with a lone `/` are the ones to reason about rather than guess. `"/" + "bin"`
keeps the first part's slash and adds no separator (there is already one); `"/" + ""` is the
root directory and must survive; `"" + "/"` has nothing in the first part and a second part
that trims to nothing, so the result is empty.

## Constraints

- **Do not call** `strcat`, `strncat`, `strlcat` or `strlcpy`. `strlen`, `memcpy` and `memset`
  are the tools. The harness inspects `pathjoin.o`.
- **Exactly one allocation per successful call.** The harness checks the live-block count
  before and after.
- **All-or-nothing**: if an allocation fails, return NULL with nothing left allocated.
- The result is exactly the right size: no slack, and no `realloc` to shrink it.
- Neither function modifies any part, and `path_join` may be written in terms of
  `path_join_n`.
- No fixed-size intermediate buffer: parts may be any length.

## What the harness checks

- **Thirteen `path_join` cases and five `path_join_n` cases**, covering every row of the table
  above, each one freed afterwards.
- **The live-block count** after all of them: every successful join was freed, so the count is
  back where it started.
- **Forced failures**: `ds_fail_nth(k)` makes the `k`th allocation from that point fail, for
  `k` from 1 to 4 on `path_join` and 1 to 6 on `path_join_n`. After each, either the call
  succeeded and allocated exactly one block (which the harness frees), or it returned NULL and
  **the live-block count is unchanged**.
- **The fault sweep** (`make fault`, part of `make check`): the whole program is run once per
  allocation site with `FAIL_AT=1`, `2`, ... Each run must either finish normally or exit with
  `EXIT_OOM` (3) after cleaning up. A run that leaks is caught by the course's leak check at
  exit, which ends the process with status 4.
- **Leaks**: `lib/dsalloc.c` reports every block still live at exit, with its allocation site,
  on every platform; LeakSanitizer does the same on Linux in the ASan build.

## Verify

```bash
make check
```

Success prints the four stages — `== forbid`, `== plain+dsalloc`, `== asan+ubsan+dsalloc+lsan`
and `== fault-sweep` — and ends with `s04-e07-path-join: all checks passed` and
`== check: OK`. The sweep line reports how many injection points it found:

```
fault-sweep: N=29 pass=11 oom=18 fail=0
```

`pass` runs completed normally (the forced failure was at a point the program never reached),
`oom` runs exited 3 as they should, and `fail` must be 0.

A leak looks like this instead, from the course's own checker:

```
dsalloc: LEAK 8 bytes allocated at pathjoin.c:72
```

and the process exits 4. The fix is always the same: on the failure path, free what you have
already allocated before returning NULL.

A wrong separator rule shows up as an ordinary check failure:

```
FAIL main.c:22: path_join
       got:  usr//bin
       want: usr/bin
```

<details><summary>Hint</summary>

Write the pass that computes the length first, and make the writing pass use the same
function.

</details>
