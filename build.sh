#!/bin/bash
# get sdl-nacl, sdl_mixer-nacl, jumpnbump-nacl and nacl-sdk
# set $SDL_PREFIX (to some temporary location) and $NACL_SDK_ROOT
echo $1

set -e

ARCH=$1

BITS=
if [ "$ARCH" -eq "64" ]; then
  BITS=64
fi

echo $BITS

export CFLAGS=-m$ARCH
export CCFLAGS=-m$ARCH
export LDFLAGS=-m$ARCH
export CC=$NACL_SDK_ROOT/toolchain/linux_x86/bin/nacl-gcc
export CXX=$NACL_SDK_ROOT/toolchain/linux_x86/bin/nacl-g++

export SDL_PREFIX=$NACL_SDK_ROOT//toolchain/linux_x86/nacl$BITS/usr

export CPPFLAGS=-I$SDL_PREFIX/include

# # Go to libmikmod dir
# cd libmikmod-nacl
# ./configure --host=nacl --prefix=$SDL_PREFIX --enable-esd=no
# make clean
# make
# make install
# cd ..


# # Go to SDL dir
# cd sdl-nacl
# ./autogen.sh
# ./configure --host=nacl64 --disable-assembly --prefix=$SDL_PREFIX
# make clean
# make
# make install
# cd ..



# # Go to SDL_mixer dir
# cd sdl_mixer-nacl
# ./autogen.sh
# ./configure --host=nacl \
#   --enable-shared=no --with-sdl-prefix=$SDL_PREFIX --enable-music-mod=yes \
#   --enable-music-mod-shared=no --enable-music-mp3=no --prefix=$SDL_PREFIX
# make clean
# make
# make install
# cd ..

# Go to jumpnbump dir
cd jumpnbump-nacl
make clean
make data
make jumpnbump.nexe
cd ..
