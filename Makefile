# Top-level targets for LearnCStringsByGrinding.
# POSIX make syntax only (works with macOS make 3.81, GNU make 4.x, bmake).
# Pass a compiler with: make check CC=clang
SH = /bin/sh

all: check

# What CI runs on every leg.
check: lint selftest verify

lint:
	@$(SH) scripts/lint.sh

selftest:
	@CC="$(CC)" $(SH) scripts/selftest.sh

verify:
	@CC="$(CC)" $(SH) scripts/verify-all.sh

# Removes stray build/ directories left by `make` inside exercises. Never touches reps/.
clean:
	@find units solutions capstones -type d -name build -prune -exec rm -rf {} + 2>/dev/null; true

.PHONY: all check lint selftest verify clean
