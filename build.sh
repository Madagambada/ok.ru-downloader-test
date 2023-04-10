#!/bin/bash

echo -n "Set toolchain vars... "
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
export LD_RUN_PATH=$LD_RUN_PATH:/usr/local/lib
export C_INCLUDE_PATH=C_INCLUDE_PATH$:/usr/local/include
export CPLUS_INCLUDE_PATH=CPLUS_INCLUDE_PATH$:/usr/local/include

echo -n "Create build directory... "
mkdir build
cd build

echo -n "Configure ok.ru-downloader... "
cmake -DCMAKE_BUILD_TYPE=Release ..

echo -n "Build ok.ru-downloader... "
make

exit 0
