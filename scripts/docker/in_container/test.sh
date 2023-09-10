#!/bin/sh
set -e -u

cmake -Bbuild/profile -S/source -DCMAKE_BUILD_TYPE=Profile
cmake --build build/profile -j 4 --target coverage_report

mkdir -p /source/build_artifacts
cd build/profile
tar cf - coverage_report | (cd /source/build_artifacts && tar xf - )
