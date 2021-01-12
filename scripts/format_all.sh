#!/usr/bin/env bash

SRC_DIR="$( dirname $(readlink -e $0) )"
cd "${SRC_DIR}/../src/" || exit 1

find . -path ./build -prune -false -o \
    -regex '.*\.\(cpp\|hpp\)' \
    -exec clang-format --verbose -i {} \;
