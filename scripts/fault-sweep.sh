#!/bin/sh
# fault-sweep.sh BIN [ARGS...] -- run by `make fault` and by `make check` for [FAULT].
# 1. Runs BIN once with FA_COUNT=1 to learn N, the number of allocation calls.
# 2. Runs BIN with FAIL_AT=1..N. Each run must exit 0 (checks pass) or 3 (EXIT_OOM),
#    with no sanitizer report and no dsalloc leak report (exit 4).
# Last line: "fault-sweep: N=<n> pass=<p> oom=<o> fail=<f>"; exit 0 iff f == 0.
set -u
here=$(cd "$(dirname "$0")" && pwd)
bin=$1
shift
: "${TIMEOUT:=20}"
unset FAIL_AT FA_COUNT
log=$(mktemp "${TMPDIR:-/tmp}/lcsg-sweep.XXXXXX") || exit 2
trap 'rm -f "$log"' EXIT
trap 'exit 130' INT TERM
report_re='ERROR: (AddressSanitizer|LeakSanitizer)|runtime error:|SUMMARY: (Address|Leak|UndefinedBehavior)Sanitizer|dsalloc: LEAK'

FA_COUNT=1 sh "$here/run-timeout.sh" "$TIMEOUT" "$bin" "$@" >"$log" 2>&1
rc=$?
if [ "$rc" -ne 0 ] || grep -qE "$report_re" "$log"; then
    echo "fault-sweep: baseline run (no injected failure) failed, exit $rc:"
    cat "$log"
    echo "fault-sweep: N=? pass=0 oom=0 fail=1"
    exit 1
fi
n=$(sed -n 's/^dsalloc: \([0-9][0-9]*\) allocation calls$/\1/p' "$log" | tail -n 1)
if [ -z "$n" ]; then
    echo "fault-sweep: no allocation count printed (was lib/dsalloc.c linked?)"
    echo "fault-sweep: N=? pass=0 oom=0 fail=1"
    exit 1
fi
[ "$n" -eq 0 ] && echo "fault-sweep: warning: program made no counted allocations"

pass=0 oom=0 fail=0 shown=0 i=1
while [ "$i" -le "$n" ]; do
    FAIL_AT=$i sh "$here/run-timeout.sh" "$TIMEOUT" "$bin" "$@" >"$log" 2>&1
    rc=$?
    verdict=
    if grep -qE "$report_re" "$log"; then
        verdict="sanitizer or leak report (exit $rc)"
    elif [ "$rc" -eq 0 ]; then
        pass=$((pass + 1))
    elif [ "$rc" -eq 3 ]; then
        oom=$((oom + 1))
    else
        verdict="exit $rc"
    fi
    if [ -n "$verdict" ]; then
        fail=$((fail + 1))
        echo "fault-sweep: FAIL_AT=$i: $verdict"
        if [ "$shown" -eq 0 ]; then
            shown=1
            grep -E "$report_re|^ *#[0-9]+ |^FAIL |dsalloc: " "$log" | head -n 15 | sed 's/^/    /'
        fi
    fi
    i=$((i + 1))
done
echo "fault-sweep: N=$n pass=$pass oom=$oom fail=$fail"
[ "$fail" -eq 0 ]
