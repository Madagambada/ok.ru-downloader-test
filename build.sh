#!/bin/bash

echo -n "Set toolchain vars... "
export TOOLCHAIN=$(pwd)/dependencies/x86_64-linux-musl-native
export AR=$TOOLCHAIN/bin/x86_64-linux-musl-gcc-ar
export AS=$TOOLCHAIN/bin/as
export CC=$TOOLCHAIN/bin/x86_64-linux-musl-gcc
export CXX=$TOOLCHAIN/bin/x86_64-linux-musl-g++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/x86_64-linux-musl-gcc-ranlib
export STRIP=$TOOLCHAIN/bin/strip

echo -n "Create build directory... "
mkdir build
cd build

echo -n "Configure tgb... "
cmake -DCMAKE_BUILD_TYPE=Release ..

echo -n "Build tgb... "
make

exit 0
