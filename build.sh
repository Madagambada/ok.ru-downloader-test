#!/bin/bash

echo -n "Set toolchain vars... "
export TOOLCHAIN=$(pwd)/dependencies/musl/musl-toolchain
export CC=$TOOLCHAIN/bin/musl-gcc

echo -n "Create build directory... "
mkdir build
cd build

echo -n "Configure ok.ru-downloader... "
cmake -DCMAKE_BUILD_TYPE=Release ..

echo -n "Build ok.ru-downloader... "
make

exit 0
