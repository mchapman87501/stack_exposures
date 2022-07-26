#!/bin/sh
set -e -u

SCRIPT_DIR=$(dirname "$0")
cd ${SCRIPT_DIR}/../

cmake -Bbuild/debug -S. \
      -DCMAKE_BUILD_TYPE=Debug
cd build/debug
cmake --build . -j 4 --target coverage_report
open coverage_report/index.html