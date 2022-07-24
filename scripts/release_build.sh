#!/bin/sh
set -e -u

cmake -Bbuild/release -S. \
      -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_INSTALL_PREFIX=${PWD}/local
cd build/release
cmake --build . -j 4
