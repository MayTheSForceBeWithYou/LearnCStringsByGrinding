#!/bin/sh
# verify-all.sh [--only PATH]... [COURSE_DIR] -- verify every exercise, drill and solution.
#
# Portable POSIX sh (dash, bash-as-sh, macOS /bin/sh); no GNU-only tool options.
# COURSE_DIR defaults to $LCSG_ROOT, else the directory above this script.
# --only PATH (repeatable) limits the run to items whose path relative to the course
# root starts with PATH, e.g. --only units/s03-copying. Mirrored solutions
# are checked with their exercise.
#
# Walks units/*/eNN-*/, units/*/drills/rNN-*/, units/*/lesson-demo/ and capstones/*/.
# The solution of units/U/X is solutions/U/X; of capstones/C, solutions/capstones/C.
#
#   WRITE / DRILL / FAULT / DESIGN starter
#       every source compiles (-Werror), the link FAILS, and the undefined symbols
#       the linker reports are exactly EXPECT_SYMBOLS ("scaffold OK"). GNU ld, lld
#       and Apple ld messages are all understood (scripts/undef-syms.sh).
#   BUG starter         builds, then `make check` fails
#   REPORT starter      builds; capture.txt source-sha256 matches the source
#   PREDICT / TRACE     builds
#   ... with answers.txt (PREDICT, LOOKUP, REPORT): `make check` also FAILS, because
#                       the shipped answers.txt is blank and the program grades it
#   solution            `make check` passes (FORBID check, plain + ASan/UBSan, dsalloc
#                       leak check everywhere, LSan on Linux, fault sweep for FAULT);
#                       the harness part of every file is byte-identical to the starter
#                       (answers.txt and ANSWER.md are the student's, so they may differ)
#   lesson-demo/        builds; capture*.txt match the source
# Anything else found under units/, capstones/ or solutions/ is logged as SKIP, by name.
#
# Every build goes to a fresh temporary directory outside the tree (B=...), which
# is removed at exit, so the working tree is never written to.
# Exit status: 0 iff nothing failed and at least one item was checked.
set -u
LC_ALL=C
export LC_ALL
here=$(cd "$(dirname "$0")" && pwd)
only=
while [ $# -gt 0 ]; do
    case "$1" in
    --only) only="$only ${2%/}"; shift 2 ;;
    --only=*) v=${1#--only=}; only="$only ${v%/}"; shift ;;
    -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
    *) break ;;
    esac
done
root=$(cd "${1:-${LCSG_ROOT:-$here/..}}" && pwd) || exit 2
: "${MAKE:=make}"
: "${ITEM_TIMEOUT:=900}"

work=$(mktemp -d "${TMPDIR:-/tmp}/lcsg-verify.XXXXXX") || exit 2
trap 'rm -rf "$work"' EXIT
trap 'exit 130' INT TERM
log=$work/log
seen=$work/seen
: >"$seen"
unset FAIL_AT FA_COUNT DSALLOC_LEAKS
fails=0 total=0 skips=0 n_scaffold=0 n_solution=0 seq=0

wanted() { # wanted REL -> 0 if no --only given or REL matches one
    [ -z "$only" ] && return 0
    for o in $only; do
        case "$1" in "$o"|"$o"/*) return 0 ;; esac
    done
    return 1
}
mkvar() { # mkvar DIR NAME -> value of NAME = ... in DIR/Makefile (single line)
    sed -n -E "s/^[[:space:]]*$2[[:space:]]*[:?+]?=[[:space:]]*//p" "$1/Makefile" | head -n 1 | sed 's/[[:space:]]*$//'
}
newout() { seq=$((seq + 1)); printf '%s/b%d' "$work" "$seq"; }
mk() { # mk DIR OUT TARGET... -> make in DIR, building into OUT
    mk_d=$1 mk_o=$2
    shift 2
    "$MAKE" --no-print-directory -C "$mk_d" B="$mk_o" WERROR=1 "$@"
}
row() { printf '%-52s %-9s %-8s %-5s %s\n' "$1" "$2" "$3" "$4" "$5"; }
result() { # result PATH TAG KIND OK DETAIL
    total=$((total + 1))
    if [ "$4" = 1 ]; then
        row "$1" "$2" "$3" PASS "$5"
    else
        row "$1" "$2" "$3" FAIL "$5"
        fails=$((fails + 1))
    fi
}
skip() { # skip PATH WHY
    skips=$((skips + 1))
    row "$1" - - SKIP "$2"
}
showlog() { sed 's/^/    | /' "$log" | tail -n "$1"; }
# shellcheck disable=SC2086 # word splitting of the TAG list is intended
kind_of() { printf '%s\n' $1 | grep -vx NO-ASAN | head -n 1; }
first_error() { grep -m1 -E 'error|Error' "$log" | cut -c1-100; }
src_hash() { "$MAKE" -s --no-print-directory -C "$1" print-src-hash 2>/dev/null; }
check_captures() { # check_captures DIR HASH -> prints the names of stale capture*.txt
    for cf in "$1"/capture*.txt; do
        [ -f "$cf" ] || continue
        want=$(sed -n 's/^# source-sha256: *\([0-9a-f]*\).*/\1/p' "$cf" | head -n 1)
        [ "$want" = "$2" ] || printf ' %s' "${cf##*/}"
    done
}

# fidelity STARTER SOLUTION TAG -> prints problems; the solution must keep the shipped
# harness: files without a marker line are identical; files with a line containing
# "below this line. ----" are identical up to and including that line.
fidelity() {
    fs=$1 fd=$2 ftag=$3
    for ff in "$fs"/*; do
        [ -f "$ff" ] || continue
        fn=${ff##*/}
        case "$fn" in README.md|ANSWER.md|answers.txt|capture*.txt) continue ;; esac
        if [ ! -f "$fd/$fn" ]; then
            printf ' missing:%s' "$fn"
            continue
        fi
        fline=$(grep -n 'below this line\. ----' "$ff" | head -n 1 | cut -d: -f1)
        if [ -n "$fline" ]; then
            head -n "$fline" "$ff" >"$work/fa"
            head -n "$fline" "$fd/$fn" >"$work/fb"
            cmp -s "$work/fa" "$work/fb" || printf ' harness-differs:%s' "$fn"
        else
            case " $ftag " in
            *" BUG "*|*" DESIGN "*|*" REPORT "*) [ "$fn" = Makefile ] || continue ;;
            esac
            # Capstone starters live one directory shallower than their
            # solutions, so the include line is ../../ vs ../../../.
            if [ "$fn" = Makefile ]; then
                sed 's#^include \(\.\./\)*common\.mk#include common.mk#' "$ff" >"$work/fa"
                sed 's#^include \(\.\./\)*common\.mk#include common.mk#' "$fd/$fn" >"$work/fb"
                cmp -s "$work/fa" "$work/fb" || printf ' differs:%s' "$fn"
            else
                cmp -s "$ff" "$fd/$fn" || printf ' differs:%s' "$fn"
            fi
        fi
    done
}

verify_solution() { # DIR REL STARTER_DIR
    d=$1 rel=$2 sdir=$3
    tag=$(mkvar "$d" TAG)
    out=$(newout)
    ok=0
    if sh "$here/run-timeout.sh" "$ITEM_TIMEOUT" "$MAKE" --no-print-directory -C "$d" B="$out" WERROR=1 check >"$log" 2>&1; then
        ok=1
        detail="check ok: $(grep '^== ' "$log" | grep -v 'check: OK' | sed 's/^== //' | tr '\n' ' ' | sed 's/ $//; s/ /, /g')"
    else
        why=$(grep -m1 -E '^fault-sweep: (FAIL_AT|baseline)|^FAIL |^forbid: |WRONG|blank$|ERROR:|SUMMARY|runtime error|dsalloc: LEAK|run-timeout:|error:' "$log" | cut -c1-80)
        detail="make check failed at '$(grep '^== ' "$log" | tail -n 1 | sed 's/^== //')': $why"
    fi
    sweep=$(sed -n 's/^fault-sweep: \(N=.*\)$/\1/p' "$log" | tail -n 1)
    [ -n "$sweep" ] && detail="$detail; sweep $sweep"
    stale=$(check_captures "$d" "$(src_hash "$d")")
    if [ -n "$stale" ]; then ok=0; detail="$detail; STALE:$stale"; fi
    if [ -n "$sdir" ]; then
        drift=$(fidelity "$sdir" "$d" "$tag")
        if [ -n "$drift" ]; then ok=0; detail="$detail; starter mismatch:$drift"; fi
    fi
    [ "$ok" = 1 ] && n_solution=$((n_solution + 1))
    result "$rel" "${tag:-?}" solution "$ok" "$detail"
    [ "$ok" = 1 ] || showlog 25
    rm -rf "$out"
}

verify_link_fail() { # DIR -> sets ok, detail
    syms=$(mkvar "$d" EXPECT_SYMBOLS)
    if [ -z "$syms" ]; then
        detail="no EXPECT_SYMBOLS in Makefile"
        return
    fi
    if ! mk "$d" "$out" build-objs >"$log" 2>&1; then
        detail="compile error (only the link may fail): $(first_error)"
        return
    fi
    if mk "$d" "$out" build >"$log" 2>&1; then
        detail="starter links, but must not (a dummy body, or EXPECT_SYMBOLS defined?)"
        return
    fi
    sh "$here/undef-syms.sh" "$log" >"$work/got"
    # shellcheck disable=SC2086 # one symbol per word
    printf '%s\n' $syms | sort -u >"$work/exp"
    if [ ! -s "$work/got" ]; then
        detail="link failed without an undefined-symbol report: $(first_error)"
        return
    fi
    missing=$(comm -23 "$work/exp" "$work/got" | tr '\n' ' ' | sed 's/ $//')
    extra=$(comm -13 "$work/exp" "$work/got" | tr '\n' ' ' | sed 's/ $//')
    if [ -n "$missing" ]; then
        detail="defined, but must be left to the student: $missing"
    elif [ -n "$extra" ]; then
        detail="undefined symbol not in EXPECT_SYMBOLS: $extra"
    else
        ok=1
        n_scaffold=$((n_scaffold + 1))
        detail="scaffold OK: link fails on own symbol(s): $(tr '\n' ' ' <"$work/got" | sed 's/ $//')"
    fi
}

# blank_answers DIR -> sets ok/detail: a starter that ships answers.txt must fail its
# own grader (every answer blank), and must not ship a filled-in answer.
blank_answers() {
    [ -f "$d/answers.txt" ] || return 0
    if grep -Eq '^[^#=]*=[[:space:]]*[^[:space:]]' "$d/answers.txt"; then
        ok=0; detail="answers.txt in a starter has a filled-in answer"
    elif sh "$here/run-timeout.sh" "$ITEM_TIMEOUT" "$MAKE" --no-print-directory -C "$d" B="$out" WERROR=1 check >"$log" 2>&1; then
        ok=0; detail="make check passes with a blank answers.txt"
    elif grep -q 'blank$' "$log"; then
        detail="$detail; blank answers.txt fails the grader"
    else
        ok=0; detail="make check fails, but not on blank answers"
    fi
}

verify_starter() { # DIR REL
    d=$1 rel=$2
    tag=$(mkvar "$d" TAG)
    kind=$(kind_of "$tag")
    out=$(newout)
    ok=0 detail=
    case "$kind" in
    WRITE|DRILL|FAULT|DESIGN)
        if cat "$d"/*.c "$d"/*.h 2>/dev/null | grep -q '^[[:space:]]*#[[:space:]]*error'; then
            detail="#error in a starter (not allowed: the link must be what fails)"
        else
            verify_link_fail
        fi ;;
    BUG)
        if ! mk "$d" "$out" build-all >"$log" 2>&1; then detail="does not build: $(first_error)"
        elif sh "$here/run-timeout.sh" "$ITEM_TIMEOUT" "$MAKE" --no-print-directory -C "$d" B="$out" WERROR=1 check >"$log" 2>&1; then
            detail="make check passes; the bug is not detected"
        else
            ok=1; detail="builds; check fails at: $(grep '^== ' "$log" | tail -n 1 | sed 's/^== //')"
            stale=$(check_captures "$d" "$(src_hash "$d")")
            if [ -n "$stale" ]; then ok=0; detail="STALE capture:$stale (source changed; re-run make capture)"; fi
        fi ;;
    REPORT)
        if ! mk "$d" "$out" build-all >"$log" 2>&1; then detail="does not build: $(first_error)"
        elif [ ! -f "$d/capture.txt" ]; then detail="no capture.txt (run: make capture TOOL=asan|run)"
        else
            stale=$(check_captures "$d" "$(src_hash "$d")")
            if [ -z "$stale" ]; then ok=1; detail="builds; capture matches source"; blank_answers
            else detail="STALE capture: source changed since capture (re-run make capture)"; fi
        fi ;;
    PREDICT|TRACE)
        if mk "$d" "$out" build-all >"$log" 2>&1; then ok=1; detail="builds"; blank_answers
        else detail="does not build: $(first_error)"; fi ;;
    *)
        detail="unknown TAG '$tag'" ;;
    esac
    result "$rel" "${tag:-?}" starter "$ok" "$detail"
    [ "$ok" = 1 ] || showlog 15
    rm -rf "$out"
}

verify_item() { # DIR REL SOLUTION_REL
    vd=$1 vrel=$2 vsrel=$3
    if [ ! -f "$vd/Makefile" ]; then
        result "$vrel" "?" starter 0 "no Makefile"
        return
    fi
    verify_starter "$vd" "$vrel"
    echo "$vsrel" >>"$seen"
    vtag=$(mkvar "$vd" TAG)
    if [ -f "$root/$vsrel/Makefile" ]; then
        verify_solution "$root/$vsrel" "$vsrel" "$vd"
    elif ls "$root/$vsrel"/capture*.txt >/dev/null 2>&1; then
        stale=$(check_captures "$root/$vsrel" "$(src_hash "$vd")")
        if [ -z "$stale" ]; then result "$vsrel" "$vtag" solution 1 "capture matches starter source"
        else result "$vsrel" "$vtag" solution 0 "STALE:$stale (re-capture from the starter)"; fi
    else
        case " $vtag " in
        *" WRITE "*|*" DRILL "*|*" FAULT "*|*" DESIGN "*|*" BUG "*)
            result "$vsrel" "$vtag" solution 0 "missing solution (no $vsrel/Makefile)" ;;
        *) if [ -f "$vd/answers.txt" ]; then
               result "$vsrel" "$vtag" solution 0 "missing solution (an answers.txt exercise needs one)"
           else
               [ -d "$root/$vsrel" ] || skip "$vsrel" "no solution dir (not required for $vtag)"
           fi ;;
        esac
    fi
}

verify_demo() { # DIR REL
    out=$(newout)
    if ! mk "$1" "$out" build-all >"$log" 2>&1; then result "$2" demo demo 0 "does not build: $(first_error)"
    else
        stale=$(check_captures "$1" "$(src_hash "$1")")
        if [ -z "$stale" ]; then result "$2" demo demo 1 "builds; captures match source"
        else result "$2" demo demo 0 "STALE:$stale"; fi
    fi
    rm -rf "$out"
}

echo "verify-all: root $root"
echo "verify-all: $(uname -s) $(uname -m); CC=${CC:-cc}: $(${CC:-cc} --version 2>/dev/null | head -n 1)"
echo "verify-all: $("$MAKE" --version 2>/dev/null | head -n 1); sh=$(command -v sh)"
case "$(uname -s)" in
Darwin) echo "verify-all: leak checks: dsalloc (all builds); LeakSanitizer not available on this platform" ;;
*) echo "verify-all: leak checks: dsalloc (all builds) + LeakSanitizer (ASan build)" ;;
esac
echo
row ITEM TAG KIND RES DETAIL
row ---------------------------------------------------- --------- -------- ----- ------

for u in "$root"/units/*; do
    [ -e "$u" ] || continue
    urel=${u#"$root"/}
    if [ ! -d "$u" ]; then wanted "$urel" && skip "$urel" "not a unit directory"; continue; fi
    for x in "$u"/* "$u"/.[!.]*; do
        [ -e "$x" ] || continue
        xrel=${x#"$root"/}
        xn=${x##*/}
        wanted "$xrel" || continue
        case "$xn" in
        LESSON.md) ;;
        e[0-9][0-9]-*)
            if [ -d "$x" ]; then verify_item "$x" "$xrel" "solutions/${xrel#units/}"
            else skip "$xrel" "not a directory"; fi ;;
        lesson-demo)
            if [ -d "$x" ]; then verify_demo "$x" "$xrel"; else skip "$xrel" "not a directory"; fi ;;
        drills)
            for r in "$x"/* "$x"/.[!.]*; do
                [ -e "$r" ] || continue
                rrel=${r#"$root"/}
                wanted "$rrel" || continue
                case "${r##*/}" in
                README.md) ;;
                r[0-9][0-9]-*)
                    if [ -d "$r" ]; then verify_item "$r" "$rrel" "solutions/${rrel#units/}"
                    else skip "$rrel" "not a directory"; fi ;;
                *) skip "$rrel" "not a drill (drills are drills/rNN-<slug>/)" ;;
                esac
            done ;;
        *) skip "$xrel" "not an exercise, drills/ or lesson-demo/ (ignored)" ;;
        esac
    done
done
for c in "$root"/capstones/*; do
    [ -e "$c" ] || continue
    crel=${c#"$root"/}
    wanted "$crel" || continue
    if [ -d "$c" ]; then verify_item "$c" "$crel" "solutions/$crel"
    else skip "$crel" "not a capstone directory"; fi
done
# Orphans and stray files in solutions/.
orphan() { # orphan PATH REL WANTED_REL
    wanted "$3" || return 0
    grep -Fqx "$2" "$seen" || result "$2" "?" solution 0 "orphan: no matching exercise"
}
for su in "$root"/solutions/*; do
    [ -e "$su" ] || continue
    surel=${su#"$root"/}
    case "${su##*/}" in
    capstones)
        for sc in "$su"/*; do
            [ -e "$sc" ] || continue
            if [ -d "$sc" ]; then orphan "$sc" "${sc#"$root"/}" "${sc#"$root"/solutions/}"
            else wanted "${sc#"$root"/solutions/}" && skip "${sc#"$root"/}" "stray file in solutions/"; fi
        done
        continue ;;
    esac
    if [ ! -d "$su" ]; then wanted "units/${surel#solutions/}" && skip "$surel" "stray file in solutions/"; continue; fi
    for sx in "$su"/*; do
        [ -e "$sx" ] || continue
        sxrel=${sx#"$root"/}
        case "${sx##*/}" in
        e[0-9][0-9]-*) orphan "$sx" "$sxrel" "units/${sxrel#solutions/}" ;;
        drills)
            for sr in "$sx"/*; do
                [ -e "$sr" ] || continue
                srrel=${sr#"$root"/}
                case "${sr##*/}" in
                r[0-9][0-9]-*) orphan "$sr" "$srrel" "units/${srrel#solutions/}" ;;
                *) wanted "units/${srrel#solutions/}" && skip "$srrel" "not a drill solution (ignored)" ;;
                esac
            done ;;
        *) wanted "units/${sxrel#solutions/}" && skip "$sxrel" "not an exercise solution (ignored)" ;;
        esac
    done
done

echo
if [ "$total" -eq 0 ]; then
    echo "verify-all: no exercises found under $root"
    exit 1
fi
echo "verify-all: scaffolds failing to link on their own symbols: $n_scaffold; solutions passing: $n_solution"
echo "verify-all: $((total - fails))/$total passed, $fails failed, $skips skipped (each listed above)  ($root)"
[ "$fails" -eq 0 ]
