#!/bin/sh
set -e -u

cd $(dirname "$0")/../

cmake -Bbuild/release -S. \
      -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=${PWD}/build/release/local
cmake --build build/release -j 4
cmake --install build/release
