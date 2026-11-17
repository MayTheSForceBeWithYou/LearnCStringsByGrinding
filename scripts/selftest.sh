#!/bin/sh
# selftest.sh -- prove the harness itself works, on this machine and toolchain.
#
# Builds a throwaway course tree in a temporary directory (common.mk, lib/ and
# scripts/ copied from this repo, plus small fixture exercises) and checks that:
#   - an untouched scaffold fails to LINK on exactly its own symbols, and verify-all
#     counts that as "scaffold OK"; adding the definitions makes it build and pass;
#   - every way a scaffold can be wrong is reported as a failure: a dummy body, a
#     partial stub, a compile error, an #error line, a foreign undefined symbol;
#   - solutions that leak, overflow, hang, drift from the shipped harness, or are
#     missing, fail; a correct FAULT solution survives the sweep and a leaky one does not;
#   - stray directories are logged as SKIP by name, orphan solutions fail;
#   - scripts/undef-syms.sh parses GNU ld, lld, and Apple ld (ld64 and ld-prime) output;
#   - lib/dsalloc.c counts, fails on demand, tolerates libc-owned pointers, and turns
#     a leak into exit status 4 on every platform;
#   - scripts/run-timeout.sh enforces limits and preserves exit statuses;
#   - nothing is written into the fixture tree by verify-all;
#   - FORBID catches a forbidden libc call (direct, through a macro, with Apple's or a
#     fortified symbol name); answers.txt exercises grade blank, wrong, misspelled and
#     correct answers and never print the right value; lib/guard catches a one-byte
#     overflow in both builds; lib/bytes, lib/oracle and the UTF-8 locale behave the
#     same everywhere; lint enforces the README and LESSON structure of docs/SPEC.md.
# Exit status 0 iff every check passed. The temporary tree is always removed.
set -u
LC_ALL=C
export LC_ALL
here=$(cd "$(dirname "$0")" && pwd)
src=$(cd "$here/.." && pwd)
: "${MAKE:=make}"
T=$(mktemp -d "${TMPDIR:-/tmp}/lcsg-selftest.XXXXXX") || exit 2
trap 'rm -rf "$T"' EXIT
trap 'exit 130' INT TERM
cp "$src/common.mk" "$T/"
cp -R "$src/lib" "$src/scripts" "$T/"
U=units/s98-selftest
S=solutions/s98-selftest
mkdir -p "$T/$U" "$T/$S"
ok=0 bad=0
pass() { ok=$((ok + 1)); echo "selftest: ok   $1"; }
fail() { bad=$((bad + 1)); echo "selftest: FAIL $1"; [ -f "$T/out" ] && sed 's/^/    | /' "$T/out" | tail -n 20; }

# mkex REL TAG SYMS [EXTRA_MAKEFILE_LINE] -- write REL/Makefile with the right include depth.
mkex() {
    mkdir -p "$T/$1"
    depth=$(printf '%s\n' "$1" | tr -cd '/' | wc -c | tr -d ' ')
    up=
    i=0
    while [ "$i" -le "$depth" ]; do up="../$up"; i=$((i + 1)); done
    {
        echo "TAG = $2"
        echo "SRCS = main.c"
        [ -n "$3" ] && echo "EXPECT_SYMBOLS = $3"
        [ -n "${4:-}" ] && echo "$4"
        echo "include ${up}common.mk"
    } >"$T/$1/Makefile"
}
# A two-function starter: the harness calls both, nothing defines them.
starter_src() {
    cat <<'EOF'
#include "course.h"
int st_answer(void);
int st_twice(int x);
int main(void)
{
    check(st_answer() == 42, "the answer");
    check(st_twice(21) == 42, "twice");
    return checks_done("s98-selftest");
}
/* ---- Your definitions go below this line. ---- */
EOF
}
good_defs() {
    cat <<'EOF'
int st_answer(void) { return 42; }
int st_twice(int x) { return 2 * x; }
EOF
}
# mkpair NAME TAG SYMS -- starter + correct solution
mkpair() {
    mkex "$U/$1" "$2" "$3" "${4:-}"
    starter_src >"$T/$U/$1/main.c"
    mkex "$S/$1" "$2" "$3" "${4:-}"
    { starter_src; good_defs; } >"$T/$S/$1/main.c"
}
verify() { sh "$T/scripts/verify-all.sh" "$@" "$T" >"$T/out" 2>&1; }
# expect NAME pass|fail REGEX ONLY-PATH...
expect() {
    en=$1 ew=$2 er=$3
    shift 3
    oargs=
    for p in "$@"; do oargs="$oargs --only $p"; done
    # shellcheck disable=SC2086 # oargs is a word list
    verify $oargs
    erc=$?
    if [ "$ew" = pass ] && [ "$erc" -ne 0 ]; then fail "$en (verify-all exited $erc, expected 0)"; return; fi
    if [ "$ew" = fail ] && [ "$erc" -eq 0 ]; then fail "$en (verify-all passed, expected a failure)"; return; fi
    if grep -Eq "$er" "$T/out"; then pass "$en"; else fail "$en (no line matching: $er)"; fi
}

# ---------------------------------------------------------------- fixtures
mkpair e01-scaffold WRITE "st_answer st_twice"

mkex "$U/e02-stub-body" WRITE "st_answer st_twice"
{ starter_src; echo 'int st_answer(void) { return 0; }'; echo 'int st_twice(int x) { return x; }'; } >"$T/$U/e02-stub-body/main.c"

mkex "$U/e03-compile-error" WRITE "st_answer st_twice"
{ starter_src; echo 'int broken(void) { return 1 }'; } >"$T/$U/e03-compile-error/main.c"

mkex "$U/e04-foreign-symbol" WRITE "st_answer"
starter_src >"$T/$U/e04-foreign-symbol/main.c"

mkex "$U/e05-partial-stub" WRITE "st_answer st_twice"
{ starter_src; echo 'int st_twice(int x) { return 2 * x; }'; } >"$T/$U/e05-partial-stub/main.c"

mkex "$U/e06-hash-error" WRITE "st_answer st_twice"
{ echo '#error write the functions below, then delete this line'; starter_src; } >"$T/$U/e06-hash-error/main.c"

mkpair e07-leaky-solution WRITE "st_answer st_twice"
{ starter_src; echo '#include <stdlib.h>'; echo 'int st_answer(void) { char *p = malloc(16); p[0] = 42; return p[0]; }'; echo 'int st_twice(int x) { return 2 * x; }'; } >"$T/$S/e07-leaky-solution/main.c"

mkpair e08-overflow-solution WRITE "st_answer st_twice"
{ starter_src; echo '#include <stdlib.h>'; echo 'int st_answer(void) { volatile int i = 4; int *p = malloc(4 * sizeof *p); if (!p) return 0; p[i] = 1; free(p); return 42; }'; echo 'int st_twice(int x) { return 2 * x; }'; } >"$T/$S/e08-overflow-solution/main.c"

mkpair e09-slow-solution WRITE "st_answer st_twice" "TIMEOUT = 2"
{ starter_src; echo 'int st_answer(void) { volatile int spin = 1; while (spin) { } return 42; }'; echo 'int st_twice(int x) { return 2 * x; }'; } >"$T/$S/e09-slow-solution/main.c"

mkpair e10-drifted-solution WRITE "st_answer st_twice"
{ starter_src | sed 's/st_twice(21) == 42/1/'; good_defs; } >"$T/$S/e10-drifted-solution/main.c"

mkex "$U/e11-missing-solution" WRITE "st_answer st_twice"
starter_src >"$T/$U/e11-missing-solution/main.c"

fault_src() {
    cat <<'EOF'
#include <stdlib.h>
#include <string.h>
#include "course.h"
char **st_three(void);
void st_free3(char **v);
int main(void)
{
    char **v = st_three();
    if (!v) {
        fprintf(stderr, "s98-selftest: out of memory\n");
        return EXIT_OOM;
    }
    check(strcmp(v[0], "a") == 0 && strcmp(v[2], "c") == 0, "three strings");
    st_free3(v);
    return checks_done("s98-selftest-fault");
}
/* ---- Your definitions go below this line. ---- */
EOF
}
fault_good() {
    cat <<'EOF'
char **st_three(void)
{
    char **v = calloc(3, sizeof *v);
    if (!v)
        return NULL;
    const char *src[3] = {"a", "b", "c"};
    for (int i = 0; i < 3; i++) {
        v[i] = strdup(src[i]);
        if (!v[i]) {
            st_free3(v);
            return NULL;
        }
    }
    return v;
}
void st_free3(char **v)
{
    if (!v)
        return;
    for (int i = 0; i < 3; i++)
        free(v[i]);
    free(v);
}
EOF
}
mkex "$U/e12-fault" FAULT "st_three st_free3"
fault_src >"$T/$U/e12-fault/main.c"
mkex "$S/e12-fault" FAULT "st_three st_free3"
{ fault_src; fault_good; } >"$T/$S/e12-fault/main.c"
mkex "$U/e13-fault-leak" FAULT "st_three st_free3"
fault_src >"$T/$U/e13-fault-leak/main.c"
mkex "$S/e13-fault-leak" FAULT "st_three st_free3"
{ fault_src; fault_good | sed 's/            st_free3(v);/            (void)0;/'; } >"$T/$S/e13-fault-leak/main.c"

mkpair drills/r01-drill DRILL "st_answer st_twice"
mkdir -p "$T/$U/notes"
echo "scratch" >"$T/$U/notes/todo.txt"
mkdir -p "$T/$S/e99-orphan"
echo "TAG = WRITE" >"$T/$S/e99-orphan/Makefile"

# ---------------------------------------------------------------- harness behaviour
find "$T/units" "$T/solutions" | sort >"$T/before"

expect "untouched scaffold fails to link on its own symbols (scaffold OK)" pass \
    "starter +PASS +scaffold OK: link fails on own symbol\(s\): st_answer st_twice" "$U/e01-scaffold"
expect "its solution passes plain + ASan/UBSan + leak check" pass \
    "e01-scaffold +WRITE +solution PASS +check ok: plain\+dsalloc, asan\+ubsan\+dsalloc" "$U/e01-scaffold"
expect "scaffold with dummy bodies is rejected" fail "starter links, but must not" "$U/e02-stub-body"
expect "scaffold with a compile error is rejected" fail "compile error \(only the link may fail\)" "$U/e03-compile-error"
expect "undefined symbol outside EXPECT_SYMBOLS is rejected" fail "not in EXPECT_SYMBOLS: st_twice" "$U/e04-foreign-symbol"
expect "partially stubbed scaffold is rejected" fail "defined, but must be left to the student: st_twice" "$U/e05-partial-stub"
expect "#error stop line is rejected" fail "#error in a starter" "$U/e06-hash-error"
expect "leaking solution fails (dsalloc, all platforms)" fail "dsalloc: LEAK" "$U/e07-leaky-solution"
expect "heap overflow in a solution fails (ASan)" fail "heap-buffer-overflow" "$U/e08-overflow-solution"
expect "hanging solution is killed by the time limit" fail "run-timeout: killed after 2s" "$U/e09-slow-solution"
expect "solution that edits the shipped harness fails" fail "starter mismatch: harness-differs:main.c" "$U/e10-drifted-solution"
expect "missing solution fails" fail "missing solution" "$U/e11-missing-solution"
expect "correct FAULT solution survives every injected failure" pass "sweep N=4 pass=0 oom=4 fail=0" "$U/e12-fault"
expect "FAULT solution that leaks on a failure path fails the sweep" fail "fault-sweep: FAIL_AT=[0-9]+: sanitizer or leak report" "$U/e13-fault-leak"
expect "drills/rNN-* are verified like exercises" pass "drills/r01-drill +DRILL +starter +PASS +scaffold OK" "$U/drills"
expect "stray directories are logged as SKIP by name" pass "units/s98-selftest/notes +- +- +SKIP" "$U/notes" "$U/e01-scaffold"
expect "orphan solutions fail" fail "e99-orphan .*orphan: no matching exercise" "$U/e99-orphan"

find "$T/units" "$T/solutions" | sort >"$T/after"
if cmp -s "$T/before" "$T/after"; then pass "verify-all wrote nothing into the tree"
else fail "verify-all left files in the tree: $(comm -13 "$T/before" "$T/after" | head -n 5 | tr '\n' ' ')"; fi
rm -f "$T/out"

# Adding the definitions to the untouched scaffold makes it build and pass.
mkdir -p "$T/scratch"
cp "$T/$U/e01-scaffold/Makefile" "$T/$U/e01-scaffold/main.c" "$T/scratch/"
sed 's#^include .*#include ../common.mk#' "$T/$U/e01-scaffold/Makefile" >"$T/scratch/Makefile"
if "$MAKE" --no-print-directory -C "$T/scratch" B="$T/scratch-b" build >"$T/out" 2>&1; then
    fail "scaffold copy linked before definitions were added"
else
    pass "scaffold copy does not link before definitions are added"
fi
good_defs >>"$T/scratch/main.c"
# Fresh build dir: make 3.81 on macOS compares whole-second mtimes, so main.o
# built in the same second as this edit would otherwise look up to date.
if "$MAKE" --no-print-directory -C "$T/scratch" B="$T/scratch-b2" WERROR=1 check >"$T/out" 2>&1 \
   && grep -q "s98-selftest: all checks passed" "$T/out"; then
    pass "adding the definitions makes the scaffold build and pass make check"
else
    fail "adding the definitions did not make the scaffold pass"
fi
rm -rf "$T/scratch-b" "$T/out"

# ---------------------------------------------------------------- linker message parsing
mkdir -p "$T/logs"
printf '%s\n' "/usr/bin/ld: main.o: in function \`main':" \
    "main.c:(.text+0x1d): undefined reference to \`ex_foo'" \
    "/usr/bin/ld: main.c:(.text+0x2d): undefined reference to \`ex_bar'" \
    "collect2: error: ld returned 1 exit status" >"$T/logs/gnu"
printf '/usr/bin/ld: main.c:9: undefined reference to \342\200\230ex_foo\342\200\231\n' >"$T/logs/gnu-utf8"
printf '%s\n' "ld.lld: error: undefined symbol: ex_foo" ">>> referenced by main.c:9" \
    "ld.lld: error: undefined symbol: ex_bar" >"$T/logs/lld"
printf '%s\n' "Undefined symbols for architecture arm64:" '  "_ex_foo", referenced from:' \
    "      _main in main.o" '  "_ex_bar", referenced from:' "      _main in main.o" \
    "ld: symbol(s) not found for architecture arm64" \
    "clang: error: linker command failed with exit code 1 (use -v to see invocation)" >"$T/logs/apple"
printf '%s\n' "Undefined symbols for architecture x86_64:" '  "_ex_foo", referenced from:' \
    "      _main in main-1a2b3c.o" "ld: symbol(s) not found for architecture x86_64" >"$T/logs/apple-x86"
printf '%s\n' "main.c:3:10: error: expected ';' after return statement" >"$T/logs/compile"
for f in gnu lld apple; do
    got=$(sh "$T/scripts/undef-syms.sh" "$T/logs/$f" | tr '\n' ' ')
    if [ "$got" = "ex_bar ex_foo " ]; then pass "undef-syms parses $f linker output"
    else fail "undef-syms on $f: got '$got'"; fi
done
for f in gnu-utf8 apple-x86; do
    got=$(sh "$T/scripts/undef-syms.sh" "$T/logs/$f" | tr '\n' ' ')
    if [ "$got" = "ex_foo " ]; then pass "undef-syms parses $f linker output"
    else fail "undef-syms on $f: got '$got'"; fi
done
got=$(sh "$T/scripts/undef-syms.sh" "$T/logs/compile")
if [ -z "$got" ]; then pass "undef-syms reports nothing for a compile error"; else fail "undef-syms on a compile error: '$got'"; fi

# ---------------------------------------------------------------- dsalloc
mkdir -p "$T/probe"
cat >"$T/probe/main.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "course.h"
int main(int argc, char **argv)
{
    /* The POSIX.1-2008 functions the course promises are declared on every platform. */
    struct timespec ts;
    check(clock_gettime(CLOCK_MONOTONIC, &ts) == 0, "clock_gettime(CLOCK_MONOTONIC) works");
    size_t base = ds_live_blocks();
    char *a = malloc(10), *b = strdup("hello"), *c = strndup("abcdef", 3);
    int *z = calloc(4, sizeof *z);
    check(a && b && c && z, "allocations succeed");
    check(ds_live_blocks() == base + 4 && ds_live_bytes() == 10 + 6 + 4 + 4 * sizeof *z, "live counts");
    check(c && strcmp(c, "abc") == 0, "strndup copies at most n");
    ds_fail_nth(1);
    char *r = realloc(a, 1000);
    check(r == NULL && ds_live_blocks() == base + 4, "failed realloc keeps the old block");
    ds_fail_nth(2);
    void *ok1 = malloc(1), *no = malloc(1);
    check(ok1 != NULL && no == NULL, "ds_fail_nth(2) fails the second call only");
    free(ok1);
    a = realloc(a, 1000);
    check(a != NULL && ds_live_bytes() == 1000 + 6 + 4 + 4 * sizeof *z, "realloc retracks the block");
    /* A pointer allocated inside libc is freed without being counted. */
    FILE *f = tmpfile();
    if (f) {
        fputs("line one\n", f);
        rewind(f);
        char *line = NULL;
        size_t cap = 0;
        check(getline(&line, &cap, f) > 0, "getline reads");
        free(line);
        fclose(f);
    }
    void (*release)(void *) = free;      /* free may be used as a function pointer */
    release(z);
    free(a); free(b); free(c);
    check(ds_live_blocks() == base, "everything freed");
    if (argc > 1 && strcmp(argv[1], "leak") == 0)
        (void)malloc(24);
    return checks_done("dsalloc-probe");
}
EOF
printf 'TAG = PREDICT\nSRCS = main.c\ninclude ../common.mk\n' >"$T/probe/Makefile"
if "$MAKE" --no-print-directory -C "$T/probe" B="$T/probe-b" WERROR=1 build-all >"$T/out" 2>&1; then
    for mode in plain asan; do
        if [ "$mode" = asan ]; then env_asan="ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:allocator_may_return_null=1"; else env_asan="DSALLOC_DUMMY=1"; fi
        env "$env_asan" FA_COUNT=1 "$T/probe-b/$mode/prog" >"$T/out" 2>&1
        rc=$?
        if [ "$rc" -eq 0 ] && grep -q "dsalloc-probe: all checks passed" "$T/out" && grep -q "^dsalloc: [0-9][0-9]* allocation calls$" "$T/out"; then
            pass "dsalloc counts, injects failures and tolerates libc pointers ($mode)"
        else fail "dsalloc probe ($mode) exit $rc"; fi
        env "$env_asan" "$T/probe-b/$mode/prog" leak >"$T/out" 2>&1
        rc=$?
        if [ "$rc" -eq 4 ] && grep -q "dsalloc: LEAK: 1 block(s), 24 byte(s)" "$T/out" && grep -q "allocated at main.c:" "$T/out"; then
            pass "dsalloc turns a leak into exit 4 with the allocation site ($mode)"
        else fail "dsalloc leak detection ($mode) exit $rc"; fi
    done
    env FAIL_AT=1 "$T/probe-b/plain/prog" >"$T/out" 2>&1
    if grep -q "FAIL .*allocations succeed" "$T/out"; then pass "FAIL_AT=1 fails the first allocation"; else fail "FAIL_AT=1 had no effect"; fi
else
    fail "dsalloc probe did not build"
fi
rm -rf "$T/probe-b"

# ---------------------------------------------------------------- lint
mkdir -p "$T/lint/units/s97-lint/e01-bad" "$T/lint/scripts"
cp "$T/scripts/lint.sh" "$T/lint/scripts/"
cat >"$T/lint/units/s97-lint/e01-bad/main.c" <<'EOF'
#include <malloc.h>
#include <stdlib.h>
/* reallocarray in a comment is fine */
static const char *msg = "typeof in a string is fine";
int *grow(int *p, size_t n) { return reallocarray(p, n, sizeof *p); }
EOF
printf 'TAG = WRITE\nSRCS = main.c\ninclude ../../../common.mk\n' >"$T/lint/units/s97-lint/e01-bad/Makefile"
sh "$T/lint/scripts/lint.sh" "$T/lint" >"$T/out" 2>&1
rc=$?
if [ "$rc" -ne 0 ] && grep -q "non-portable header: malloc.h" "$T/out" \
   && grep -q "forbidden identifier(s): reallocarray " "$T/out" \
   && grep -q "TAG WRITE needs EXPECT_SYMBOLS" "$T/out" && grep -q "no README.md" "$T/out" \
   && ! grep -q "typeof" "$T/out"; then
    pass "lint rejects Linux-only headers, glibc-only calls and incomplete exercises"
else
    fail "lint did not report the planted problems (exit $rc)"
fi
rm -f "$T/out"

# ---------------------------------------------------------------- strings course: FORBID
# A reimplementation exercise: the student's file must not call the functions in FORBID.
forbid_main() {
    cat <<'EOF'
#include <string.h>
#include "course.h"
#include "strcheck.h"
char *st_copy(char *dst, const char *src);
int main(void)
{
    char *dst = guard_new(6);
    check(st_copy(dst, "hello") == dst, "returns dst");
    check_bytes(dst, "hello", 6, "copies the string and its terminator");
    check_guard(dst, "stays inside the buffer");
    guard_free(dst);
    return checks_done("s98-selftest-forbid");
}
EOF
}
# mkforbid NAME BODY-OF-st_copy.c
mkforbid() {
    for base in "$U" "$S"; do
        mkdir -p "$T/$base/$1"
        printf 'TAG = WRITE\nSRCS = main.c st_copy.c\nEXPECT_SYMBOLS = st_copy\nFORBID = strcpy memcpy\nSTUDENT_SRCS = st_copy.c\ninclude ../../../common.mk\n' >"$T/$base/$1/Makefile"
        forbid_main >"$T/$base/$1/main.c"
        printf '#include <string.h>\nchar *st_copy(char *dst, const char *src);\n/* ---- Your definitions go below this line. ---- */\n' >"$T/$base/$1/st_copy.c"
    done
    printf '%s\n' "$2" >>"$T/$S/$1/st_copy.c"
}
mkforbid e20-forbid-ok 'char *st_copy(char *dst, const char *src) { char *d = dst; while ((*d++ = *src++) != 0) { } return dst; }'
mkforbid e21-forbid-call 'char *st_copy(char *dst, const char *src) { return strcpy(dst, src); }'
mkforbid e22-forbid-macro '#define COPY(d, s, n) memcpy((d), (s), (n))
char *st_copy(char *dst, const char *src) { COPY(dst, src, strlen(src) + 1); return dst; }'
expect "FORBID: a loop that calls nothing forbidden passes, with a forbid stage" pass \
    "e20-forbid-ok +WRITE +solution PASS +check ok: forbid, plain" "$U/e20-forbid-ok"
expect "FORBID: a solution that calls the forbidden function fails" fail \
    "forbid: st_copy.o calls strcpy\(\)" "$U/e21-forbid-call"
expect "FORBID: a forbidden call hidden in a macro fails" fail \
    "forbid: st_copy.o calls memcpy\(\)" "$U/e22-forbid-macro"
printf '%s\n' "                 U _strcpy" "                 U ___memcpy_chk" "0000000000000000 T _st_copy" >"$T/nm.out"
printf '#!/bin/sh\ncat "%s"\n' "$T/nm.out" >"$T/fake-nm"
chmod +x "$T/fake-nm"
NM="$T/fake-nm" sh "$T/scripts/forbid.sh" "strcpy memcpy strlen" x.o >"$T/out" 2>&1
rc=$?
if [ "$rc" -eq 1 ] && grep -q "calls strcpy()" "$T/out" && grep -q "calls memcpy()" "$T/out" && ! grep -q "strlen()" "$T/out"; then
    pass "forbid.sh understands Apple's leading underscore and fortified (_chk) names"
else fail "forbid.sh on Apple-style nm output (exit $rc)"; fi

# ---------------------------------------------------------------- strings course: answers.txt
predict_main() {
    cat <<'EOF'
#include <string.h>
#include "answers.h"
int main(int argc, char **argv)
{
    char buf[4];
    memset(buf, '#', sizeof buf);
    buf[0] = 'a';
    buf[1] = '\0';
    answers_load(argc > 1 ? argv[1] : "answers.txt");
    answer_bytes("one.buf", buf, sizeof buf);
    answer_int("one.len", (long)strlen(buf));
    answer_hash("one.word", answers_hash("one.word=yes"));
    return answers_done("s98-selftest-predict");
}
EOF
}
# mkpredict NAME STARTER-ANSWERS SOLUTION-ANSWERS ("-" = no solution)
mkpredict() {
    mkdir -p "$T/$U/$1"
    printf 'TAG = PREDICT\nSRCS = main.c\ninclude ../../../common.mk\n' >"$T/$U/$1/Makefile"
    predict_main >"$T/$U/$1/main.c"
    printf '%s\n' "$2" >"$T/$U/$1/answers.txt"
    [ "$3" = "-" ] && return
    mkdir -p "$T/$S/$1"
    cp "$T/$U/$1/Makefile" "$T/$U/$1/main.c" "$T/$S/$1/"
    printf '%s\n' "$3" >"$T/$S/$1/answers.txt"
}
blank='# comment
one.buf =
one.len =
one.word ='
right='one.buf = a\0##
one.len = 1
one.word = YES'
mkpredict e30-predict "$blank" "$right"
mkpredict e31-predict-wrong "$blank" 'one.buf = a\0\0\0
one.len = 1
one.word = yes'
mkpredict e32-predict-filled "$right" "$right"
mkpredict e33-predict-typo "$blank" "$right
one.lenn = 1"
mkpredict e34-predict-nosolution "$blank" -
expect "answers.txt: a blank starter fails its own grader, which is what a starter must do" pass \
    "e30-predict +PREDICT +starter +PASS +builds; blank answers.txt fails the grader" "$U/e30-predict"
expect "answers.txt: correct answers pass (bytes, integer, hashed word, any case)" pass \
    "e30-predict +PREDICT +solution PASS" "$U/e30-predict"
expect "answers.txt: a wrong answer fails, and is named" fail "one.buf +WRONG" "$U/e31-predict-wrong"
expect "answers.txt: a starter that ships a filled-in answer is rejected" fail \
    "answers.txt in a starter has a filled-in answer" "$U/e32-predict-filled"
expect "answers.txt: a misspelled key is reported" fail "one.lenn +unknown key" "$U/e33-predict-typo"
expect "answers.txt: an exercise with answers.txt needs a solution" fail \
    "missing solution \(an answers.txt exercise needs one\)" "$U/e34-predict-nosolution"
verify --only "$U/e31-predict-wrong"
if grep -q 'a\\0##' "$T/out"; then fail "the grader printed the correct value"; else pass "the grader never prints the correct value"; fi
rm -f "$T/out"

# ---------------------------------------------------------------- strings course: lib probes
mkdir -p "$T/sprobe"
cat >"$T/sprobe/main.c" <<'EOF'
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include "course.h"
#include "oracle.h"
#include "strcheck.h"
#include "utf8locale.h"
int main(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";
    char out[64];

    /* Byte notation. */
    check(bytes_notation("a\0\n\t\\\xE9#", 7, out, sizeof out) == 14, "bytes_notation returns the full length");
    check(strcmp(out, "a\\0\\n\\t\\\\\\xE9#") == 0, "bytes_notation renders NUL, escapes and high bytes");
    check(bytes_notation("abcdef", 6, out, 4) == 6 && strcmp(out, "abc") == 0, "bytes_notation truncates like snprintf");
    check(sign_of(-7) == -1 && sign_of(0) == 0 && sign_of(3) == 1, "sign_of");

    /* Guarded buffers. */
    char *g = guard_new(8);
    check(memcmp(g, "########", 8) == 0 && guard_cap(g) == 8, "guard_new gives cap bytes of #");
    memcpy(g, "12345678", 8);
    check(guard_ok(g), "writing all cap bytes is fine");
    guard_reset(g);
    check(g[0] == '#' && g[7] == '#', "guard_reset");
    char *z = guard_new(0);
    check(guard_ok(z), "a zero-capacity buffer exists");
    char *c = guard_copy("abc", 3);
    check(memcmp(c, "abc", 3) == 0 && guard_cap(c) == 3, "guard_copy");
    if (strcmp(mode, "overflow") == 0) {
        volatile size_t i = 8;
        g[i] = 'X'; /* one byte too many */
        printf("guard_ok after overflow: %d\n", guard_ok(g));
    }
    if (strcmp(mode, "zero") == 0) {
        volatile size_t i = 0;
        z[i] = 'X'; /* a zero-capacity buffer must not be touched */
        printf("guard_ok after zero write: %d\n", guard_ok(z));
    }
    guard_free(c);
    guard_free(z);
    guard_free(g);

    /* The generator is the same everywhere. */
    orc_seed(42);
    check(orc_next() == 1083814273u, "the LCG's first value after seed 42");
    char s[17];
    orc_seed(7);
    size_t total = 0, empties = 0;
    for (int i = 0; i < 800; i++) {
        size_t n = orc_string(s, 16, ORC_ALPHA_AB);
        check(n <= 16 && strlen(s) == n && strspn(s, "ab") == n, "orc_string stays in its alphabet and length");
        total += n;
        empties += n == 0;
    }
    check(orc_case() == 800 && empties > 50 && total > 4000, "orc_string produces empty and long strings");

    /* What the course promises from POSIX is declared under the course flags. */
    char line[16];
    check(stpcpy(line, "ab") == line + 2 && stpncpy(line, "ab", 4) == line + 2, "stpcpy, stpncpy");
    check(strnlen("abc", 2) == 2 && strcasecmp("AbC", "aBc") == 0, "strnlen, strcasecmp");
    char *save = NULL;
    strcpy(line, "a,b");
    check(strcmp(strtok_r(line, ",", &save), "a") == 0, "strtok_r");
    FILE *f = fmemopen("x\ny\n", 4, "r");
    check(f != NULL && fgets(line, sizeof line, f) != NULL && strcmp(line, "x\n") == 0, "fmemopen");
    if (f)
        fclose(f);

    /* A UTF-8 locale exists and decodes a two-byte character. */
    const char *loc = course_utf8_locale();
    wchar_t wc = 0;
    mbstate_t st;
    memset(&st, 0, sizeof st);
    check(loc != NULL && mbrtowc(&wc, "\xC3\xA9", 2, &st) == 2 && wc == 0xE9, "course_utf8_locale selects a UTF-8 locale");
    return checks_done("strings-probe");
}
EOF
printf 'TAG = PREDICT\nSRCS = main.c\ninclude ../common.mk\n' >"$T/sprobe/Makefile"
if "$MAKE" --no-print-directory -C "$T/sprobe" B="$T/sprobe-b" WERROR=1 build-all >"$T/out" 2>&1; then
    asan_env="ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:halt_on_error=1"
    for mode in plain asan; do
        env "$asan_env" "$T/sprobe-b/$mode/prog" >"$T/out" 2>&1
        rc=$?
        if [ "$rc" -eq 0 ] && grep -q "strings-probe: all checks passed" "$T/out"; then
            pass "bytes, guard, oracle, POSIX string functions and the UTF-8 locale work ($mode)"
        else fail "strings probe ($mode) exit $rc"; fi
    done
    "$T/sprobe-b/plain/prog" overflow >"$T/out" 2>&1
    if grep -q "guard_ok after overflow: 0" "$T/out"; then pass "guard: one byte past the capacity is detected (plain build)"
    else fail "guard did not detect a one-byte overflow (plain)"; fi
    env ASAN_OPTIONS=detect_leaks=0 "$T/sprobe-b/asan/prog" overflow >"$T/out" 2>&1
    rc=$?
    if [ "$rc" -ne 0 ] && grep -q "heap-buffer-overflow" "$T/out" && grep -q "0 bytes after 8-byte region" "$T/out"; then
        pass "guard: one byte past the capacity is an exact ASan report (ASan build)"
    else fail "guard overflow under ASan (exit $rc)"; fi
    "$T/sprobe-b/plain/prog" zero >"$T/out" 2>&1
    if grep -q "guard_ok after zero write: 0" "$T/out"; then pass "guard: a write to a zero-capacity buffer is detected (plain build)"
    else fail "guard did not detect a write to a zero-capacity buffer (plain)"; fi
    env ASAN_OPTIONS=detect_leaks=0 "$T/sprobe-b/asan/prog" zero >"$T/out" 2>&1
    rc=$?
    if [ "$rc" -ne 0 ] && grep -q "use-after-poison" "$T/out"; then
        pass "guard: a write to a zero-capacity buffer is an ASan report (ASan build)"
    else fail "guard zero-capacity write under ASan (exit $rc)"; fi
else
    fail "strings probe did not build"
fi
rm -rf "$T/sprobe-b"

# ---------------------------------------------------------------- strings course: lint
L=$T/slint
mkdir -p "$L/units/s96-lint/e01-bad" "$L/units/s96-lint/drills/r01-bad" "$L/scripts"
cp "$T/scripts/lint.sh" "$L/scripts/"
printf 'TAG = WRITE\nSRCS = main.c x.c\nEXPECT_SYMBOLS = my_len\nFORBID = strlen\ninclude ../../../common.mk\n' >"$L/units/s96-lint/e01-bad/Makefile"
printf 'size_t my_len(const char *s);\n' >"$L/units/s96-lint/e01-bad/x.h"
printf '#include <stddef.h>\nsize_t strlen(const char *s)\n{\n    return s[0] != 0;\n}\n' >"$L/units/s96-lint/e01-bad/x.c"
printf 'int main(void) { return 0; }\n' >"$L/units/s96-lint/e01-bad/main.c"
cat >"$L/units/s96-lint/e01-bad/README.md" <<'EOF'
# s96-e01-bad `[WRITE]` (S)
## Goal
> **This starter does not build yet, on purpose.**
## Constraints
## The API
    size_t my_len(char *s);
## Verify
EOF
printf 'TAG = DRILL\nSRCS = main.c\nEXPECT_SYMBOLS = d_one\ninclude ../../../../common.mk\n' >"$L/units/s96-lint/drills/r01-bad/Makefile"
printf 'int d_one(void);\nint main(void) { return d_one(); }\n' >"$L/units/s96-lint/drills/r01-bad/main.c"
printf '# drill\n## Skill\n> does not build yet, on purpose\n## Rules\n## Target\n## Verify\n' >"$L/units/s96-lint/drills/r01-bad/README.md"
cat >"$L/units/s96-lint/LESSON.md" <<'EOF'
# S96
## Organizer
## The functions in this unit
| Function | Header | Standard | Card | Contract |
|---|---|---|---|---|
| `strfoo` | `<string.h>` | ISO C | below | nothing |
## Foundations
EOF
sh "$L/scripts/lint.sh" "$L" >"$T/out" 2>&1
rc=$?
if [ "$rc" -ne 0 ] && grep -q "defines a libc function name" "$T/out" \
   && grep -q "FORBID needs STUDENT_SRCS" "$T/out" \
   && grep -q "e01-bad/README.md: missing section: ## Background" "$T/out" \
   && grep -q "does not show the header's prototype: size_t my_len(const char \*s);" "$T/out" \
   && grep -q "r01-bad/README.md: missing section: ## Functions" "$T/out" \
   && grep -q "LESSON.md: missing section: ## Function reference" "$T/out" \
   && grep -q "no reference card (### \`strfoo\`)" "$T/out" \
   && grep -q "tables do not list e01-bad/" "$T/out"; then
    pass "lint enforces README sections, prototypes, FORBID, libc names and lesson cards"
else
    fail "lint did not report the planted strings-course problems (exit $rc)"
fi
rm -f "$T/out"

# ---------------------------------------------------------------- run-timeout
sh "$T/scripts/run-timeout.sh" 1 sleep 10 >"$T/out" 2>&1
rc=$?
if [ "$rc" -eq 124 ] && grep -q "killed after 1s" "$T/out"; then pass "run-timeout kills at the limit with status 124"; else fail "run-timeout limit: exit $rc"; fi
sh "$T/scripts/run-timeout.sh" 5 sh -c 'exit 7' >"$T/out" 2>&1
rc=$?
if [ "$rc" -eq 7 ]; then pass "run-timeout preserves the exit status"; else fail "run-timeout status: $rc"; fi
sh "$T/scripts/run-timeout.sh" 5 sh -c 'kill -ABRT $$' >"$T/out" 2>&1
rc=$?
if [ "$rc" -eq 134 ]; then pass "run-timeout reports SIGABRT as 134"; else fail "run-timeout SIGABRT: $rc"; fi
rm -f "$T/out"

echo
echo "selftest: $ok passed, $bad failed"
[ "$bad" -eq 0 ]
