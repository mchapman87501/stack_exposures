#!/bin/sh
set -e -u

cmake -Bbuild/debug -S/source -DCMAKE_BUILD_TYPE=Debug
cd build/debug
cmake --build . -j 4 --target coverage_report

mkdir -p /source/build_artifacts
tar cf - coverage_report | (cd /source/build_artifacts && tar xf - )
