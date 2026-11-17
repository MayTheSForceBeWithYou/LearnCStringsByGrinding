#!/bin/sh
# lint.sh [COURSE_DIR] -- static checks every PR must pass (run by CI on every leg).
#
#   portability   no Linux-only headers, no glibc/BSD-only functions, no GNU C
#                 extensions, no feature-test macros other than _POSIX_C_SOURCE,
#                 in units/, solutions/ and capstones/ (docs/AUTHORING.md section 9)
#   makefiles     every exercise, drill and solution Makefile has a valid TAG, an
#                 EXPECT_SYMBOLS line where the tag needs one, and includes common.mk
#   files         every exercise/drill has README.md (with the build-failure notice
#                 when it must not link); every solution has NOTES.md
#   markers       no OUTPUT-PENDING markers; no CR (CRLF) characters in text files
#   artifacts     git tracks no build output (build/, *.o, *.dSYM, prog, a.out, ...)
#   scripts       scripts/*.sh are #!/bin/sh and avoid the common bashisms
#   strings       no course file defines a libc function name; FORBID comes with
#                 STUDENT_SRCS; READMEs have the sections of docs/SPEC.md 8.4 in order
#                 and show the header's prototypes; lessons have the sections of 8.1,
#                 250-550 lines, a reference card for every function listed as
#                 "below", and tables that name every exercise and drill folder
# Missing LESSON.md is a warning (lessons and exercises land in separate PRs).
set -u
LC_ALL=C
export LC_ALL
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "${1:-$here/..}" && pwd) || exit 2
cd "$root" || exit 2
bad=0 warn=0
err() { bad=$((bad + 1)); echo "lint: FAIL $1: $2"; }
wrn() { warn=$((warn + 1)); echo "lint: warn $1: $2"; }
mkvar() { sed -n -E "s/^[[:space:]]*$2[[:space:]]*[:?+]?=[[:space:]]*//p" "$1" | head -n 1 | sed 's/[[:space:]]*$//'; }

srcs=$(find units solutions capstones -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null | sort)

# --- portability ------------------------------------------------------------
hdr_re='^[[:space:]]*#[[:space:]]*include[[:space:]]*<(malloc\.h|features\.h|linux/|sys/sysinfo\.h|mcheck\.h|alloca\.h|execinfo\.h|error\.h|sys/epoll\.h|bits/|gnu/|obstack\.h|argp\.h|sys/cdefs\.h|malloc/)'
fn_re='(^|[^A-Za-z0-9_])(malloc_usable_size|malloc_size|reallocarray|strdupa|strndupa|mtrace|muntrace|mallinfo2?|malloc_trim|memalign|pvalloc|valloc|qsort_r|qsort_s|strlcpy|strlcat|memrchr|rawmemchr|strchrnul|mempcpy|stpncpy_chk|asprintf|vasprintf|alloca|arc4random[a-z_]*|explicit_bzero|typeof|__typeof__|__auto_type|_GNU_SOURCE|_DEFAULT_SOURCE|_BSD_SOURCE|_DARWIN_C_SOURCE|_XOPEN_SOURCE|__attribute__|__builtin_[A-Za-z0-9_]*|__asm__)([^A-Za-z0-9_]|$)'
for f in $srcs; do
    # Strip // and single-line /* */ comments and string literals before matching identifiers.
    code=$(sed -E -e 's#//.*##' -e 's#/\*.*\*/##g' -e 's/"([^"\\]|\\.)*"/""/g' "$f")
    if grep -Eq "$hdr_re" "$f"; then err "$f" "non-portable header: $(grep -Eo "$hdr_re" "$f" | head -n 1 | sed 's/.*<//')"; fi
    hit=$(printf '%s\n' "$code" | grep -Eo "$fn_re" | sed 's/^[^A-Za-z0-9_]//; s/[^A-Za-z0-9_]$//' | sort -u | tr '\n' ' ')
    [ -n "$hit" ] && err "$f" "non-portable or forbidden identifier(s): $hit(docs/AUTHORING.md section 9)"
    if printf '%s\n' "$code" | grep -q '({'; then err "$f" "GNU statement expression '({'"; fi
    if printf '%s\n' "$code" | grep -Eq 'case[^:]*[^.]\.\.\.[^.]'; then err "$f" "GNU case range 'case a ... b'"; fi
    if grep -Eq '^[[:space:]]*#[[:space:]]*define[[:space:]]+_POSIX_C_SOURCE[[:space:]]' "$f" \
       && ! grep -Eq '^[[:space:]]*#[[:space:]]*define[[:space:]]+_POSIX_C_SOURCE[[:space:]]+200809L[[:space:]]*$' "$f"; then
        err "$f" "_POSIX_C_SOURCE must be exactly 200809L (dsalloc.h already defines it)"
    fi
done

# --- makefiles and files ----------------------------------------------------
dirs=$( { find units -type d \( -name 'e[0-9][0-9]-*' -o -name 'r[0-9][0-9]-*' \) 2>/dev/null
          find capstones -mindepth 1 -maxdepth 1 -type d 2>/dev/null; } | sort)
for d in $dirs; do
    case "$d" in units/*/e*|units/*/drills/r*|capstones/*) ;; *) continue ;; esac
    for m in "$d" "solutions/${d#units/}"; do
        case "$d" in capstones/*) [ "$m" = "$d" ] || m="solutions/$d" ;; esac
        [ -d "$m" ] || continue
        mf=$m/Makefile
        if [ ! -f "$mf" ]; then err "$m" "no Makefile"; continue; fi
        tag=$(mkvar "$mf" TAG)
        # shellcheck disable=SC2086 # the TAG is a word list
        kinds=$(printf '%s\n' $tag | grep -vx NO-ASAN)
        nk=$(printf '%s\n' "$kinds" | grep -c .)
        [ "$nk" -ge 1 ] || err "$mf" "no TAG"
        for k in $kinds; do
            case "$k" in WRITE|DRILL|BUG|REPORT|PREDICT|TRACE|FAULT|DESIGN) ;; *) err "$mf" "unknown TAG word '$k'" ;; esac
        done
        # shellcheck disable=SC2086
        first=$(printf '%s\n' $kinds | head -n 1)
        case " $tag " in
        *" WRITE "*|*" DRILL "*|*" FAULT "*|*" DESIGN "*)
            [ -n "$(mkvar "$mf" EXPECT_SYMBOLS)" ] || err "$mf" "TAG $first needs EXPECT_SYMBOLS" ;;
        esac
        grep -Eq '^include[[:space:]]+(\.\./)+common\.mk[[:space:]]*$' "$mf" || err "$mf" "must end with: include ../…/common.mk"
        case "$d" in units/*/drills/*) want=$(printf '%s' "../../../../common.mk") ;; units/*) want=../../../common.mk ;; *) want= ;; esac
        case "$m" in solutions/capstones/*) want=../../../common.mk ;; capstones/*) want=../../common.mk ;; esac
        [ -n "$want" ] && ! grep -q "^include $want\$" "$mf" && err "$mf" "include path should be $want"
    done
    if [ ! -f "$d/README.md" ]; then err "$d" "no README.md"
    else
        case " $(mkvar "$d/Makefile" TAG 2>/dev/null) " in
        *" WRITE "*|*" DRILL "*|*" FAULT "*|*" DESIGN "*)
            grep -q 'does not build yet, on purpose' "$d/README.md" || err "$d/README.md" "missing the standard build-failure notice" ;;
        esac
    fi
    case "$d" in capstones/*) sd=solutions/$d ;; *) sd=solutions/${d#units/} ;; esac
    [ -d "$sd" ] && [ -f "$sd/Makefile" ] && [ ! -f "$sd/NOTES.md" ] && err "$sd" "no NOTES.md"
done
for u in units/*/; do
    [ -d "$u" ] || continue
    [ -f "${u}LESSON.md" ] || wrn "${u%/}" "no LESSON.md yet"
done

# --- strings course: names, FORBID, README and LESSON structure --------------
# No course file defines a function that libc defines (reimplementations are my_<name>).
libc_re='^[A-Za-z_][A-Za-z0-9_ ]*[ *](strlen|strnlen|strcpy|strncpy|stpcpy|stpncpy|strcat|strncat|strcmp|strncmp|strcoll|strxfrm|strchr|strrchr|strstr|strpbrk|strspn|strcspn|strtok|strtok_r|strerror|strdup|strndup|strcasecmp|strncasecmp|memcpy|memmove|memset|memcmp|memchr|strtol|strtoul|strtoll|strtoull|strtod|atoi|atol|getline|snprintf|sprintf|isalpha|isdigit|isspace|isupper|islower|toupper|tolower)[[:space:]]*\('
for f in $srcs; do
    hit=$(grep -En "$libc_re" "$f" | grep -v ';[[:space:]]*$' | head -n 1)
    [ -n "$hit" ] && err "$f" "defines a libc function name (use my_<name>): $hit"
done

# has_sections FILE HEADING... -> the headings must all be present, in this order
has_sections() {
    hs_f=$1
    shift
    hs_last=0
    for hs_h in "$@"; do
        hs_n=$(grep -n -x -F "$hs_h" "$hs_f" | head -n 1 | cut -d: -f1)
        if [ -z "$hs_n" ]; then err "$hs_f" "missing section: $hs_h"; return; fi
        if [ "$hs_n" -le "$hs_last" ]; then err "$hs_f" "section out of order: $hs_h"; return; fi
        hs_last=$hs_n
    done
}
for d in $dirs; do
    mf=$d/Makefile
    [ -f "$mf" ] || continue
    tag=$(mkvar "$mf" TAG)
    if [ -n "$(mkvar "$mf" FORBID)" ] && [ -z "$(mkvar "$mf" STUDENT_SRCS)" ]; then
        err "$mf" "FORBID needs STUDENT_SRCS (the files it applies to)"
    fi
    rd=$d/README.md
    [ -f "$rd" ] || continue
    case "$d" in capstones/*) continue ;; esac
    lines=$(wc -l <"$rd" | tr -d ' ')
    case "$d" in
    units/*/drills/*)
        has_sections "$rd" "## Skill" "## Functions" "## Rules" "## Target" "## Verify" ;;
    *)
        case " $tag " in
        *" WRITE "*|*" FAULT "*)
            has_sections "$rd" "## Goal" "## Background" "## The API" "## Behaviour" "## Constraints" "## What the harness checks" "## Verify"
            [ "$lines" -ge 60 ] || err "$rd" "only $lines lines; a WRITE/FAULT README is 60-150 (docs/SPEC.md 8.4)" ;;
        *" DESIGN "*)
            has_sections "$rd" "## Goal" "## Background" "## The API" "## Questions your contract must answer" "## Behaviour" "## Constraints" "## What the harness checks" "## Verify"
            [ "$lines" -ge 60 ] || err "$rd" "only $lines lines; a DESIGN README is 60-150 (docs/SPEC.md 8.4)" ;;
        *" BUG "*)
            has_sections "$rd" "## Goal" "## Background" "## Symptom" "## What the code is supposed to do" "## What to hand in" "## Verify"
            [ -f "$d/ANSWER.md" ] || err "$d" "a BUG exercise ships an ANSWER.md template" ;;
        *)
            has_sections "$rd" "## Goal" "## Background" "## Verify" ;;
        esac ;;
    esac
    grep -q '<summary>Hint</summary>' "$rd" && [ "$(grep -c '<summary>Hint</summary>' "$rd")" -gt 1 ] && err "$rd" "more than one hint"
    # The prototypes the README shows are the ones the header declares.
    for sym in $(mkvar "$mf" EXPECT_SYMBOLS); do
        proto=$(grep -h -E "[ *]$sym\(.*\);[[:space:]]*\$" "$d"/*.h 2>/dev/null | head -n 1)
        if [ -z "$proto" ]; then err "$d" "no one-line prototype for $sym in a header"; continue; fi
        grep -q -F -x "$proto" "$rd" || err "$rd" "does not show the header's prototype: $proto"
    done
done
for u in units/*/; do
    l=${u}LESSON.md
    [ -f "$l" ] || continue
    has_sections "$l" "## Organizer" "## The functions in this unit" "## Foundations" "## Function reference" "## What the tools say" "## Distinctions worth keeping straight" "## \`man\` pointers" "## Check yourself" "## Takeaways" "## Exercises" "## Drills"
    grep -q '^## Worked example: ' "$l" || err "$l" "no '## Worked example: ...' section"
    grep -q '^\*\*The plausible wrong reading\.\*\*' "$l" || err "$l" "no paragraph starting **The plausible wrong reading.**"
    lines=$(wc -l <"$l" | tr -d ' ')
    if [ "$lines" -lt 250 ] || [ "$lines" -gt 550 ]; then err "$l" "$lines lines; a lesson is 250-550 (docs/SPEC.md 8.1)"; fi
    # Every function the table says has its card "below" has one.
    for fn in $(sed -n '/^## The functions in this unit/,/^## Foundations/p' "$l" | grep -E '\|[[:space:]]*below[[:space:]]*\|' | sed -E 's/^\|[[:space:]]*`([A-Za-z0-9_]+)`.*/\1/'); do
        grep -q -x -F "### \`$fn\`" "$l" || err "$l" "no reference card (### \`$fn\`) for a function listed as 'below'"
    done
    # The Exercises and Drills tables name every folder, and nothing else.
    for x in "$u"e[0-9][0-9]-*/ "$u"drills/r[0-9][0-9]-*/; do
        [ -d "$x" ] || continue
        xn=${x%/}
        xn=${xn#"$u"}
        grep -q -F "\`$xn/\`" "$l" || err "$l" "the Exercises/Drills tables do not list $xn/"
    done
done

# --- markers ------------------------------------------------------------------
pend=$(grep -rln 'OUTPUT-PENDING' units capstones solutions 2>/dev/null | sort)
for f in $pend; do err "$f" "OUTPUT-PENDING marker (capture the real output first)"; done
txt=$(find . -path ./.git -prune -o -type f \( -name '*.c' -o -name '*.h' -o -name '*.md' -o -name '*.sh' -o -name 'Makefile' -o -name '*.mk' -o -name '*.yml' -o -name '*.txt' \) -print | sort)
cr=$(printf '\r')
for f in $txt; do
    if grep -q "$cr" "$f"; then err "$f" "contains CR characters (save with LF line endings)"; fi
done

# --- artifacts ----------------------------------------------------------------
if [ -d .git ] && command -v git >/dev/null 2>&1; then
    art=$(git ls-files | grep -E '(^|/)build/|\.(o|a|so|dylib|exe|out|obj)$|\.dSYM/|(^|/)(prog|a\.out|core)$|(^|/)vgcore\.' )
    for f in $art; do err "$f" "build artifact is tracked by git"; done
fi

# --- scripts --------------------------------------------------------------------
for f in scripts/*.sh; do
    [ "$(head -n 1 "$f")" = "#!/bin/sh" ] || err "$f" "first line must be #!/bin/sh"
    if sed 's/#.*//' "$f" | grep -Eq '\[\[[[:space:]]|^[[:space:]]*(declare|local|function|mapfile|readarray|shopt)[[:space:]]|[[:space:]]<\(|\$\{[A-Za-z_]+(,,|\^\^)'; then
        err "$f" "bash-only syntax in a POSIX sh script"
    fi
done

echo "lint: $bad problem(s), $warn warning(s)"
[ "$bad" -eq 0 ]
