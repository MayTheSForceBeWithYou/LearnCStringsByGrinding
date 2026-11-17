# common.mk -- shared build rules for every LearnCStringsByGrinding exercise.
#
# An exercise (or drill, or solution) Makefile sets a few variables and includes this file:
#
#     TAG = WRITE
#     SRCS = main.c
#     EXPECT_SYMBOLS = my_strcpy my_stpcpy
#     include ../../../common.mk
#
# Portable on purpose: GNU make 3.81 (the make macOS ships) or newer, POSIX sh,
# gcc or clang on Linux, Apple clang on macOS. See docs/AUTHORING-INFRA.md.
#
# Variables an exercise may set (all optional except TAG):
#   TAG             WRITE DRILL BUG REPORT PREDICT TRACE FAULT DESIGN; add NO-ASAN to skip ASan
#   SRCS            C sources of the program (default: main.c)
#   EXPECT_SYMBOLS  functions the student must define (WRITE/DRILL/FAULT/DESIGN starters)
#   FORBID          libc functions the student's code must not call (checked with nm -u)
#   STUDENT_SRCS    the sources FORBID applies to (required when FORBID is set); they are
#                   compiled with -fno-builtin so a call in the source is a call in the object
#   ARGS            command-line arguments passed to the program
#   EXTRA_CFLAGS    extra compile flags
#   LDLIBS          extra libraries (e.g. -lm)
#   ASAN_EXTRA      extra ASAN_OPTIONS (e.g. detect_stack_use_after_return=1)
#   TIMEOUT         seconds each program run may take (default 20)
#
# Variables the harness (scripts/verify-all.sh) sets:
#   B               build directory (default: build, inside the exercise; the harness
#                   points it at a temporary directory outside the tree)
#   WERROR=1        add -Werror
#
# Targets: make / make run, make asan, make fault, make check, make clean,
#          make valgrind (Linux only, optional), make capture TOOL=asan|run (REPORT),
#          make build-objs (compile only; used by the harness).

COURSE_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
LIBDIR      := $(COURSE_ROOT)/lib
SCRIPTS     := $(COURSE_ROOT)/scripts

TAG            ?= WRITE
SRCS           ?= main.c
EXPECT_SYMBOLS ?=
FORBID         ?=
STUDENT_SRCS   ?=
CAPTURE        ?= capture.txt
WARN_SRC       ?=
ARGS           ?=
PROG           ?= prog
TIMEOUT        ?= 20
B              ?= build
WERROR         ?=
ASAN_EXTRA     ?=

UNAME_S := $(shell uname -s)

WARN := -Wall -Wextra -Wpedantic
ifeq ($(WERROR),1)
WARN += -Werror
endif

# Every course translation unit gets lib/dsalloc.h first (allocation accounting).
# -U_FORTIFY_SOURCE: every platform runs the same libc entry points (some distributions
# turn fortification on by default); S05 turns it on deliberately through EXTRA_CFLAGS.
CFLAGS   := -std=c17 $(WARN) -g -U_FORTIFY_SOURCE -I$(LIBDIR) -include $(LIBDIR)/dsalloc.h $(EXTRA_CFLAGS)
LIBFLAGS := -std=c17 $(WARN) -g -U_FORTIFY_SOURCE -I$(LIBDIR)
# The course library, linked into every program.
LIBMODS  := dsalloc bytes guard oracle answers utf8locale
SANFLAGS := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

# LeakSanitizer exists on Linux only; Apple clang on arm64 rejects detect_leaks=1.
# The portable leak check is lib/dsalloc.c, which runs in every build on every OS.
ifeq ($(UNAME_S),Darwin)
DETECT_LEAKS := detect_leaks=0
ASAN_STAGE   := asan+ubsan+dsalloc
else
DETECT_LEAKS := detect_leaks=1
ASAN_STAGE   := asan+ubsan+dsalloc+lsan
endif
ASAN_OPTS := $(DETECT_LEAKS):abort_on_error=1:halt_on_error=1:allocator_may_return_null=1
ifneq ($(ASAN_EXTRA),)
ASAN_OPTS := $(ASAN_OPTS):$(ASAN_EXTRA)
endif
ASAN_ENV := ASAN_OPTIONS=$(ASAN_OPTS) UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1

RUN := sh $(SCRIPTS)/run-timeout.sh $(TIMEOUT)
VALGRIND := valgrind --leak-check=full --show-leak-kinds=definite,indirect,possible \
            --errors-for-leak-kinds=definite,indirect,possible --error-exitcode=1

BD         := $(abspath $(B))
PLAIN_BIN  := $(BD)/plain/$(PROG)
ASAN_BIN   := $(BD)/asan/$(PROG)
PLAIN_OBJS := $(patsubst %.c,$(BD)/plain/%.o,$(SRCS)) $(patsubst %,$(BD)/plain/_%.o,$(LIBMODS))
ASAN_OBJS  := $(patsubst %.c,$(BD)/asan/%.o,$(SRCS)) $(patsubst %,$(BD)/asan/_%.o,$(LIBMODS))
STUDENT_PLAIN_OBJS := $(patsubst %.c,$(BD)/plain/%.o,$(STUDENT_SRCS))
STUDENT_ASAN_OBJS  := $(patsubst %.c,$(BD)/asan/%.o,$(STUDENT_SRCS))
ifneq ($(FORBID),)
$(STUDENT_PLAIN_OBJS) $(STUDENT_ASAN_OBJS): CFLAGS += -fno-builtin
endif
HDRS       := $(wildcard *.h) $(wildcard $(LIBDIR)/*.h)

USE_ASAN  := $(if $(filter NO-ASAN,$(TAG)),,yes)
USE_FAULT := $(if $(USE_ASAN),$(filter FAULT,$(TAG)))

.DEFAULT_GOAL := run
.PHONY: run build build-objs build-asan build-all asan fault forbid warn check clean valgrind capture print-src-hash
.DELETE_ON_ERROR:

$(BD)/plain/%.o: %.c $(HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(BD)/asan/%.o: %.c $(HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SANFLAGS) -c $< -o $@
$(BD)/plain/_%.o: $(LIBDIR)/%.c $(wildcard $(LIBDIR)/*.h)
	@mkdir -p $(dir $@)
	$(CC) $(LIBFLAGS) -c $< -o $@
$(BD)/asan/_%.o: $(LIBDIR)/%.c $(wildcard $(LIBDIR)/*.h)
	@mkdir -p $(dir $@)
	$(CC) $(LIBFLAGS) $(SANFLAGS) -c $< -o $@

$(PLAIN_BIN): $(PLAIN_OBJS)
	$(CC) $(PLAIN_OBJS) -o $@ $(LDLIBS)
$(ASAN_BIN): $(ASAN_OBJS)
	$(CC) $(SANFLAGS) $(ASAN_OBJS) -o $@ $(LDLIBS)

build-objs: $(PLAIN_OBJS)
build: $(PLAIN_BIN)
build-asan: $(ASAN_BIN)
build-all: build $(if $(USE_ASAN),build-asan)

run: $(PLAIN_BIN)
	$(RUN) $(PLAIN_BIN) $(ARGS)

# abort_on_error=1 makes ASan die by SIGABRT; the run reports 134 and make fails.
asan: $(ASAN_BIN)
	$(ASAN_ENV) $(RUN) $(ASAN_BIN) $(ARGS)

fault: $(ASAN_BIN)
	@$(ASAN_ENV) TIMEOUT=$(TIMEOUT) sh $(SCRIPTS)/fault-sweep.sh $(ASAN_BIN) $(ARGS)

# FORBID: the student's objects (plain build) must not reference the listed libc functions.
forbid: $(STUDENT_PLAIN_OBJS)
ifneq ($(FORBID),)
	@sh $(SCRIPTS)/forbid.sh "$(FORBID)" $(STUDENT_PLAIN_OBJS)
endif

check: build-all
ifneq ($(FORBID),)
	@echo "== forbid"
	@sh $(SCRIPTS)/forbid.sh "$(FORBID)" $(STUDENT_PLAIN_OBJS)
endif
	@echo "== plain+dsalloc"
	@$(RUN) $(PLAIN_BIN) $(ARGS)
ifneq ($(USE_ASAN),)
	@echo "== $(ASAN_STAGE)"
	@$(ASAN_ENV) $(RUN) $(ASAN_BIN) $(ARGS)
endif
ifneq ($(USE_FAULT),)
	@echo "== fault-sweep"
	@$(ASAN_ENV) TIMEOUT=$(TIMEOUT) sh $(SCRIPTS)/fault-sweep.sh $(ASAN_BIN) $(ARGS)
endif
	@echo "== check: OK"

# Optional, Linux only: memcheck on the plain build. Not part of `make check`
# (Valgrind does not run on Apple Silicon).
valgrind: $(PLAIN_BIN)
	DSALLOC_LEAKS=0 $(VALGRIND) $(PLAIN_BIN) $(ARGS)

clean:
	rm -rf $(BD)

# --- captures (REPORT exercises and lesson demos) ----------------------------
# make capture TOOL=run|asan|warn [CAPTURE=capture-name.txt] [ARGS=...]
#   run, asan   the program's output in that build
#   warn        what the compiler says about $(WARN_SRC) at -O2 (never linked or run)
TOOL ?= asan
HASHED := $(sort $(SRCS) $(wildcard *.h) $(WARN_SRC))
warn:
	$(CC) -std=c17 $(WARN) -O2 -c $(WARN_SRC) -o /dev/null
print-src-hash:
	@sh $(SCRIPTS)/srchash.sh $(HASHED)
capture: $(if $(filter warn,$(TOOL)),,build-all)
	@{ echo "# source-sha256: $$(sh $(SCRIPTS)/srchash.sh $(HASHED))"; \
	   echo "# tool: $(TOOL)   captured: $$(date '+%Y-%m-%d %H:%M %Z') on $$(uname -s) $$(uname -m), $$($(CC) --version | head -n 1)"; \
	   echo "# Addresses, PIDs and thread ids will differ on your run."; \
	   $(MAKE) -s --no-print-directory $(TOOL) ARGS="$(ARGS)" 2>&1; echo "# exit status: $$?"; } > $(CAPTURE); \
	 echo "wrote $(CAPTURE)"
