#!/bin/sh
# forbid.sh "NAME NAME ..." OBJECT... -- fail if an object file calls a forbidden function.
#
# A reimplementation exercise lists, in FORBID, the libc functions the student's code
# must not call (the one being rewritten and its close relatives). common.mk compiles
# the student's sources with -fno-builtin and runs this on the resulting objects.
#
# Reads the undefined symbols of each object with `nm -u`. Understands the leading
# underscore Apple's toolchain adds (_strcpy) and the fortified entry points
# (__strcpy_chk, ___strcpy_chk). Exit status: 0 nothing forbidden, 1 a forbidden
# call was found, 2 usage or tool error.
set -u
LC_ALL=C
export LC_ALL
if [ $# -lt 2 ]; then
    echo "usage: forbid.sh \"NAME ...\" OBJECT..." >&2
    exit 2
fi
names=$1
shift
: "${NM:=nm}"
bad=0
for obj in "$@"; do
    if ! syms=$("$NM" -u "$obj" 2>/dev/null); then
        echo "forbid: cannot read symbols of $obj (is nm installed?)" >&2
        exit 2
    fi
    # Last field of each line; strip leading underscores and a _chk suffix.
    used=$(printf '%s\n' "$syms" | awk 'NF { print $NF }' | sed -e 's/@.*//' -e 's/^__*\(.*\)_chk$/\1/' -e 's/^_//' | sort -u)
    for n in $names; do
        if printf '%s\n' "$used" | grep -qx "$n"; then
            echo "forbid: ${obj##*/} calls $n(), which this exercise forbids (see Constraints in the README)"
            bad=1
        fi
    done
done
[ "$bad" -eq 0 ] || exit 1
echo "forbid: none of [$names] is called"
