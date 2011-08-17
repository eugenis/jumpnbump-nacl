#!/bin/bash

set -e

HERE=`dirname "$0"`

rm -f $HERE/app/jumpnbump-x86_32.nexe $HERE/app/jumpnbump-x86_64.nexe

STRIP32=${NACL_TOOLCHAIN_ROOT}/bin/nacl-strip
STRIP64=${NACL_TOOLCHAIN_ROOT}/bin/nacl64-strip


make -C $HERE clean
$HERE/build.sh 32 && $STRIP32 $HERE/jumpnbump.nexe && cp $HERE/jumpnbump.nexe $HERE/app/jumpnbump_x86-32.nexe

make -C $HERE clean
$HERE/build.sh 64 && $STRIP64 $HERE/jumpnbump.nexe && cp $HERE/jumpnbump.nexe $HERE/app/jumpnbump_x86-64.nexe
