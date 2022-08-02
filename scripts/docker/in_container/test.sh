#!/bin/sh
set -e -u

cmake -Bbuild/debug -S/source -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug -j 4 --target coverage_report

mkdir -p /source/build_artifacts
tar cf - build/debug/coverage_report | (cd /source/build_artifacts && tar xf - )
