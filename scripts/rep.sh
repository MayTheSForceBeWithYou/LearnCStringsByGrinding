#!/bin/sh
# rep.sh PATH -- start a fresh repetition of an exercise or drill.
#
#   sh scripts/rep.sh units/s03-copying/drills/r01-bounded-copy-reps
#
# Copies the pristine starter into reps/<unit>/<item>/<NN>/ (reps/ is gitignored),
# so every rep starts from the shipped scaffold, your earlier reps stay intact for
# comparison, and the tracked course files are never edited. Then:
#   cd reps/<unit>/<item>/<NN> && make check
set -u
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/.." && pwd)
if [ $# -ne 1 ]; then
    echo "usage: sh scripts/rep.sh units/<unit>/<eNN-...|drills/rNN-...>" >&2
    exit 2
fi
rel=${1%/}
rel=${rel#./}
case "$rel" in
units/*/drills/r[0-9][0-9]-*|units/*/e[0-9][0-9]-*) ;;
*) echo "rep: not an exercise or drill path: $rel" >&2; exit 2 ;;
esac
src=$root/$rel
[ -f "$src/Makefile" ] || { echo "rep: no Makefile in $rel" >&2; exit 2; }
unit=$(printf '%s\n' "$rel" | cut -d/ -f2)
item=${rel##*/}
n=1
while :; do
    nn=$(printf '%02d' "$n")
    dest=$root/reps/$unit/$item/$nn
    [ -e "$dest" ] || break
    n=$((n + 1))
done
mkdir -p "$dest"
for f in "$src"/*; do
    [ -f "$f" ] || continue
    case "${f##*/}" in ANSWER.md|capture*.txt) continue ;; esac
    cp "$f" "$dest/"
done
sed 's#^include .*common\.mk[[:space:]]*$#include ../../../../common.mk#' "$src/Makefile" >"$dest/Makefile"
echo "rep: fresh copy in reps/$unit/$item/$nn"
echo "rep: cd reps/$unit/$item/$nn && make check"
