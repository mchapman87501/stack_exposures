#!/bin/sh
set -e -u

cmake -Bbuild/debug -S. \
      -DCMAKE_BUILD_TYPE=Debug
cd build/debug
cmake --build . -j 4 --target coverage_report
open coverage_report/index.html