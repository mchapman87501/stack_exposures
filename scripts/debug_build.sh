#!/bin/sh
set -e -u

cd $(dirname "$0")/../

cmake -Bbuild/debug -S. \
      -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug -j 4 --target coverage_report
open build/debug/coverage_report/index.html