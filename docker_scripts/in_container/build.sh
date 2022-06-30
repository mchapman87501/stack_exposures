#!/bin/sh
set -e -u


cd
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release /source
cmake --build .

mkdir -p /source/build_artifacts
cp stack_exposures /source/build_artifacts
