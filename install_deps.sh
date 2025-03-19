#!/bin/bash
set -ex

if !command -v pkg-config &> /dev/null; then
    echo "pkg-config is not installed"
    exit 1
fi

: > deps.mk

DEPENDENCIES=("luajit")

for dep in "${DEPENDENCIES[@]}"; do
    if pkg-config --exists $dep; then
        CFLAGS="CFLAGS += $(pkg-config --cflags $dep)"
        echo "$CFLAGS" >> deps.mk
    else
        echo "pkg-config unable to find $dep"
        exit 1
    fi
done
