#!/bin/sh
set -e -u


cd
mkdir -p build/release
cd build/release

cmake -DCMAKE_BUILD_TYPE=Release /source
cmake --build .

# Prove that we got a usable executable:
./stack_exposures --help

mkdir -p /source/build_artifacts
cp stack_exposures /source/build_artifacts
