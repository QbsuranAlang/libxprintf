#!/bin/bash
#
# APPLE LICENSE
#


set -e

prefix=/tmp/xprintf

export CC=gcc
export CXX=g++
export CPP=cpp
export AR=ar
export RANLIB=ranlib
export STRIP=strip
export LD=ld
export LINK=$CXX

export CFLAGS="-ggdb -g2 -O2"

./configure \
    --prefix=${prefix} \
    $*
