#!/bin/bash
set -ex

if !command -v pkgconf &> /dev/null; then
    echo "pkg-config is not installed"
    exit 1
fi

: > deps.mk

DEPENDENCIES=("luajit")

for dep in "${DEPENDENCIES[@]}"; do
    if pkg-config --exists $dep; then
        LDFLAGS="LDFLAGS += $(pkg-config --cflags --libs $dep)"
        echo "$LDFLAGS" >> deps.mk
        make
    else
        echo "pkg-config unable to find $dep"
        exit 1
    fi
done
