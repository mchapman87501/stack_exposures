#!/bin/sh
set -e -u

mkdir -p build/release
cd build/release

cmake -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	  -DCMAKE_INSTALL_PREFIX=${PWD}/local \
	  ../..
cmake --build .
