#!/bin/bash

. system.inc

cd bzip2-1.0.5/
make -j install PREFIX=$PWD/../dist CFLAGS="$SYSFLAGS -O3"
make clean
cd ..
