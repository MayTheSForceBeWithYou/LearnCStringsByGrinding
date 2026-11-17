#!/bin/sh
# srchash.sh FILE... -- print one sha256 over the concatenation of FILEs.
# Portable: sha256sum (Linux), shasum -a 256 (macOS), or openssl.
set -u
if command -v sha256sum >/dev/null 2>&1; then
    cat "$@" | sha256sum | cut -d' ' -f1
elif command -v shasum >/dev/null 2>&1; then
    cat "$@" | shasum -a 256 | cut -d' ' -f1
else
    cat "$@" | openssl dgst -sha256 | sed 's/^.*= *//'
fi
