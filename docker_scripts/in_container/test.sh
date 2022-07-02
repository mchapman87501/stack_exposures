#!/bin/sh
set -e -u


cd
mkdir -p build/debug
cd build/debug

cmake -DCMAKE_BUILD_TYPE=Debug /source
cmake --build . --target coverage_report


mkdir -p /source/build_artifacts
tar cf - coverage_report | (cd /source/build_artifacts && tar xf - )
