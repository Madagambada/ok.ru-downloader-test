#!/bin/bash

echo -n "Set toolchain vars... "
export AR=/usr/bin/llvm-ar-16
export AS=/usr/bin/llvm-as-16
export CC=/usr/bin/clang-16
export CXX=/usr/bin/clang++-16
export LD=/usr/bin/llvm-ld-16
export RANLIB=/usr/bin/llvm-ranlib-16
export STRIP=/usr/bin/llvm-strip-16
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
export LD_RUN_PATH=$LD_RUN_PATH:/usr/local/lib
export C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/local/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/usr/local/include

echo -n "Create build directory... "
mkdir build
cd build

echo -n "Configure ok.ru-downloader... "
cmake -DCMAKE_BUILD_TYPE=Release ..

echo -n "Build ok.ru-downloader... "
make

exit 0
