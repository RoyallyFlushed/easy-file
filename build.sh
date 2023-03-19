#!/bin/sh

set -xe

CC="clang"
CFLAGS="-Wall -Wextra -pedantic -ggdb -I."

$CC $CFLAGS $OPT_FLAGS -o main main.c
