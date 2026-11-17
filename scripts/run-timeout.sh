#!/bin/sh
# run-timeout.sh SECS CMD [ARGS...] -- run CMD with a time limit, portably.
#
# macOS has no `timeout`, so the primary implementation is a few lines of perl
# (present on macOS and on Arch/Ubuntu). It forks CMD, waits, and SIGKILLs it when
# the limit expires. CMD stays in the caller's process group, so a program that
# reads the terminal still works under `make run`.
# Exit status: CMD's own status; 128+N if CMD died from signal N (134 = SIGABRT,
# how ASan and UBSan stop); 124 if the time limit killed it (one line on stderr).
# Fallbacks, in order: GNU/BSD `timeout` or `gtimeout`, then a plain sh watchdog.
set -u
if [ $# -lt 2 ]; then
    echo "usage: run-timeout.sh SECS CMD [ARGS...]" >&2
    exit 2
fi
secs=$1
shift
if command -v perl >/dev/null 2>&1; then
    exec perl -e '
        use strict; use warnings; use POSIX ();
        my $secs = shift @ARGV;
        my $pid = fork();
        die "run-timeout: fork failed: $!\n" unless defined $pid;
        if ($pid == 0) {
            no warnings "exec";
            exec { $ARGV[0] } @ARGV or do {
                print STDERR "run-timeout: cannot run $ARGV[0]: $!\n";
                POSIX::_exit(127);
            };
        }
        my $stop = sub { kill "KILL", $pid; };
        $SIG{INT}  = sub { $stop->(); exit 130 };
        $SIG{TERM} = sub { $stop->(); exit 143 };
        $SIG{ALRM} = sub {
            $stop->(); waitpid($pid, 0);
            print STDERR "run-timeout: killed after ${secs}s (time limit)\n";
            exit 124;
        };
        alarm $secs;
        waitpid($pid, 0);
        my $st = $?;
        alarm 0;
        exit(($st & 127) ? 128 + ($st & 127) : ($st >> 8));
    ' "$secs" "$@"
fi
for t in timeout gtimeout; do
    if command -v "$t" >/dev/null 2>&1; then
        "$t" "$secs" "$@"
        rc=$?
        [ "$rc" -eq 124 ] && echo "run-timeout: killed after ${secs}s (time limit)" >&2
        exit "$rc"
    fi
done
# Last resort: background watchdog.
"$@" &
pid=$!
( sleep "$secs"; kill -9 "$pid" 2>/dev/null && echo "run-timeout: killed after ${secs}s (time limit)" >&2 ) &
dog=$!
wait "$pid"
rc=$?
kill "$dog" 2>/dev/null
[ "$rc" -eq 137 ] && rc=124
exit "$rc"
