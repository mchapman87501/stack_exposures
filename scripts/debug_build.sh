#!/bin/sh
set -e -u

mkdir -p build/debug
cd build/debug

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../..
cmake --build . --target coverage_report
open coverage_report/index.html