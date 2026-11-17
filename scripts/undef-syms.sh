#!/bin/sh
# undef-syms.sh LOGFILE -- print the undefined symbols a failed link reported,
# one per line, sorted and unique, without the Mach-O leading underscore.
#
# Understands:
#   GNU ld (bfd), gold:  main.c:(.text+0x1d): undefined reference to `ex_foo'
#                        (the quotes may be UTF-8 curly quotes in some locales)
#   LLVM lld:            ld.lld: error: undefined symbol: ex_foo
#   Apple ld (ld64 and the Xcode 15+ linker):
#                        Undefined symbols for architecture arm64:
#                          "_ex_foo", referenced from:
set -u
LC_ALL=C
export LC_ALL
log=$1
{
    sed -n 's/.*undefined reference to [^A-Za-z0-9_]*\([A-Za-z_][A-Za-z0-9_]*\).*/\1/p' "$log"
    sed -n 's/.*undefined symbol: *\([A-Za-z_][A-Za-z0-9_]*\).*/\1/p' "$log"
    if grep -q 'Undefined symbols for architecture' "$log"; then
        sed -n 's/^[[:space:]]*"_\([A-Za-z_][A-Za-z0-9_]*\)", referenced from.*/\1/p' "$log"
    fi
} | sort -u
