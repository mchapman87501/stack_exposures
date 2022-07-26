#!/bin/sh
set -e -u

SCRIPT_DIR=$(dirname "$0")
cd ${SCRIPT_DIR}/../

cmake -Bbuild/release -S. \
      -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_INSTALL_PREFIX=${PWD}/local
cd build/release
cmake --build . -j 4
