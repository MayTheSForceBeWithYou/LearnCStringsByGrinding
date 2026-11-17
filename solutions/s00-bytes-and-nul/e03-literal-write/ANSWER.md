# s00-e03-literal-write: answer

## The defect

`prefix.c`, line 30, in `prefix_write`:

```c
    upcase(name);
```

`name` comes from `level_name(level)`, which returns a pointer to a string literal
(`"info"`, `"warning"` or `"error"`). `prefix.c` is at fault. `upcase.c` is innocent: it does
exactly what `upcase.h` documents, in place, on the bytes it was given.

## The mechanism

The bytes of `"warning"` are an array of eight bytes (seven characters and a terminator)
created by the compiler, with static storage duration: they exist from the start of the
program to its end, and they are part of the executable file. The linker puts them in a
section the loader maps read-only — `.rodata` on Linux, `__TEXT,__cstring` on macOS — because
nothing in the program is supposed to write there, and because identical literals can then be
shared.

`upcase` walks the string and, for every byte in `'a'..'z'`, stores the upper-case byte back
at the same address. The first letter of `"warning"` is `w`, so the very first iteration
executes a store into a read-only page. The memory management unit refuses it, the kernel
sends `SIGSEGV`, and the process dies inside `upcase`, one instruction into work it was asked
to do on memory it should never have been handed.

Note what the type system did not do. A string literal has type `char[8]` in C, so
`char *name = "warning";` is legal and silent; the array decays to `char *` like any other
array. Only the storage is read-only, and storage is not part of the type.

## How the tool showed it

The signal is `SIGSEGV`; ASan prints `SEGV on unknown address` because the address belongs to
neither the heap nor the stack nor a global it tracks, so it has nothing to say about "N bytes
after" anything and prints no allocation stack. `The signal is caused by a WRITE memory
access` identifies it as a store, which rules out a missing terminator (those are reads) and
points at the one statement in `upcase` that writes.

Frame `#0` is `upcase.c:14`, the assignment `s[i] = ...`. That is where the instruction is,
and it is not the defect: a report names the instruction that touched the memory, never the
decision that gave it the wrong memory. Frame `#1` is `prefix.c:30`, the call, and that is the
defect. The habit this builds: read the frame list from the top until you reach a line that
*chose* something.

The plain build says only `Error 139` because the program never got far enough to print a
check result, and 139 is 128 + 11: killed by signal 11.

Even where the store had been allowed to succeed — an embedded target with no protection, or
a toolchain that puts literals in writable memory — the last pair of checks in `main.c` would
have failed:

```
check_str(prefix_name(PREFIX_WARN), "warning", "prefix_name still says \"warning\" afterwards");
```

because the literal would now hold `WARNING`, permanently, for every caller in the program.
That is the defect's real shape: a function documented as read-only quietly editing data
shared by the whole program. The crash is only how this machine chooses to report it.

## The rule you take away

A string literal is the compiler's array, not yours: point at it with `const char *`, copy it
into your own buffer before changing a byte of it.
