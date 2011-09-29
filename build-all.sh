#!/bin/bash

set -e

rm -f app/jumpnbump-x86_32.nexe app/jumpnbump-x86_64.nexe

STRIP32=${NACL_TOOLCHAIN_ROOT}/bin/nacl-strip
STRIP64=${NACL_TOOLCHAIN_ROOT}/bin/nacl64-strip


make clean
make data
export ARCH=i686-nacl
make
${NACL_TOOLCHAIN_ROOT}/bin/${ARCH}-strip jumpnbump.nexe
mv jumpnbump.nexe app/jumpnbump_x86-32.nexe

make clean
make data
export ARCH=x86_64-nacl
make
${NACL_TOOLCHAIN_ROOT}/bin/${ARCH}-strip jumpnbump.nexe
mv jumpnbump.nexe app/jumpnbump_x86-64.nexe

